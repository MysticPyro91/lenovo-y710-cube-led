// SPDX-License-Identifier: MIT
/*
 * y710-led: control the Y logo and bottom light strips of a Lenovo
 * IdeaCentre Y710 Cube on Linux, the job Lenovo Nerve Center does on Windows.
 *
 * Both lights are switched by a firmware SMI: port 0xB2, EAX=0x0B, EBX=0x10
 * (set, ECX = light value) or EBX=0x11 (get, both lights packed in ECX).
 * See PROTOCOL.md for the values and how they were found.
 */
#define _GNU_SOURCE
#include <ctype.h>
#include <inttypes.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/io.h>
#include <unistd.h>

#if !defined(__x86_64__) && !defined(__i386__)
#error "y710-led requires Linux on x86"
#endif

#define VERSION "1.0.0"
#define SMI_PORT 0xb2
#define SMI_LED 0x0b
#define LED_SET 0x10
#define LED_GET 0x11
#define LIGHT2 0x1000
#define SMI_ERROR UINT32_C(0xc0000000)

static const char *const levels[] = {"off", "low", "medium", "high"};
static const uint32_t values[] = {0x0000, 0x0001, 0x0010, 0x0011};

static void usage(FILE *stream)
{
    fprintf(stream, "Usage: y710-led [--force] get\n"
                    "       y710-led [--force] set LOGO BOTTOM\n"
                    "       y710-led --help | --version\n"
                    "Levels: off | low | medium | high\n"
                    "LOGO is the Y logo, BOTTOM the light strips at the bottom of the case.\n"
                    "Needs root. Refuses to run unless DMI reports a Lenovo Y710 Cube;\n"
                    "--force skips that check (only for a Y710 Cube with odd DMI data).\n");
}

static int parse_level(const char *arg)
{
    for (int i = 0; i < 4; ++i)
        if (strcmp(arg, levels[i]) == 0)
            return i;
    return -1;
}

static int read_dmi(const char *field, char *buf, size_t size)
{
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/dmi/id/%s", field);
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;
    if (!fgets(buf, (int)size, f))
        buf[0] = '\0';
    fclose(f);
    buf[strcspn(buf, "\n")] = '\0';
    for (char *p = buf; *p; ++p)
        *p = (char)tolower((unsigned char)*p);
    return 0;
}

/* The SMI command is model-specific firmware behaviour; never send it elsewhere. */
static int is_y710_cube(void)
{
    char vendor[64], version[128], family[128];
    if (read_dmi("sys_vendor", vendor, sizeof(vendor)) == -1)
        return 0;
    if (read_dmi("product_version", version, sizeof(version)) == -1)
        version[0] = '\0';
    if (read_dmi("product_family", family, sizeof(family)) == -1)
        family[0] = '\0';
    return strcmp(vendor, "lenovo") == 0 &&
           (strstr(version, "y710 cube") != NULL || strstr(family, "y710 cube") != NULL);
}

/* Only LED_SET and LED_GET are ever passed here. No raw command access. */
static int smi(uint32_t command, uint32_t *value, const char *operation)
{
    uint32_t eax = SMI_LED, ebx = command, ecx = *value, edx = SMI_PORT;

    __asm__ volatile ("outb %%al, %%dx"
                      : "+a" (eax), "+b" (ebx), "+c" (ecx), "+d" (edx)
                      : : "memory", "cc");
    if (eax & SMI_ERROR) {
        fprintf(stderr, "y710-led: %s: firmware returned error bits 0x%08" PRIx32
                        " (eax=0x%08" PRIx32 ", ecx=0x%08" PRIx32 ")\n",
                operation, eax & SMI_ERROR, eax, ecx);
        return -1;
    }
    *value = ecx;
    return 0;
}

static const char *decode(uint32_t value, uint32_t selector)
{
    for (int i = 0; i < 4; ++i)
        if (value == (selector | values[i]))
            return levels[i];
    return "unknown";
}

int main(int argc, char **argv)
{
    int force = 0, logo = -1, bottom = -1;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage(stdout);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("y710-led " VERSION);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--force") == 0) {
        force = 1;
        --argc;
        ++argv;
    }
    int get = argc == 2 && strcmp(argv[1], "get") == 0;
    if (argc == 4 && strcmp(argv[1], "set") == 0) {
        logo = parse_level(argv[2]);
        bottom = parse_level(argv[3]);
    }
    if (!get && (logo < 0 || bottom < 0)) {
        fprintf(stderr, "y710-led: invalid arguments\n");
        usage(stderr);
        return 2;
    }
    if (!force && !is_y710_cube()) {
        fprintf(stderr, "y710-led: this machine does not report itself as a Lenovo Y710 Cube "
                        "(/sys/class/dmi/id); refusing to send the firmware command.\n");
        return 1;
    }
    if (geteuid() != 0) {
        fprintf(stderr, "y710-led: needs root; run with sudo.\n");
        return 1;
    }

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    if (sched_setaffinity(0, sizeof(cpus), &cpus) == -1) {
        perror("y710-led: cannot pin to CPU 0");
        return 1;
    }
    if (ioperm(SMI_PORT, 1, 1) == -1) {
        perror("y710-led: cannot access port 0xB2 (kernel lockdown?)");
        return 1;
    }

    int status = 0;
    uint32_t raw = 0;
    if (get) {
        if (smi(LED_GET, &raw, "get") == -1) {
            status = 1;
        } else {
            uint32_t light1 = raw & UINT32_C(0x1111);
            uint32_t light2 = raw >> 16;
            const char *name1 = decode(light1, 0);
            const char *name2 = decode(light2, LIGHT2);
            printf("logo: %s (0x%04" PRIx32 ")\n"
                   "bottom: %s (0x%04" PRIx32 ")\n"
                   "raw: 0x%08" PRIx32 "\n", name1, light1, name2, light2, raw);
            if (strcmp(name1, "unknown") == 0 || strcmp(name2, "unknown") == 0) {
                fprintf(stderr, "y710-led: unrecognized LED state; see raw value\n");
                status = 1;
            }
        }
    } else {
        raw = values[logo];
        if (smi(LED_SET, &raw, "set logo") == -1) {
            status = 1;
        } else {
            raw = LIGHT2 | values[bottom];
            if (smi(LED_SET, &raw, "set bottom") == -1) {
                fprintf(stderr, "y710-led: logo was already set; bottom may be unchanged\n");
                status = 1;
            }
        }
    }
    if (ioperm(SMI_PORT, 1, 0) == -1) {
        perror("y710-led: cannot revoke port access");
        status = 1;
    }
    return status;
}
