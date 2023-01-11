#ifndef GETOPT_H
#define GETOPT_H

/* Values for option.has_arg */
#define no_argument 0       /* The option does not take an argument. */
#define required_argument 1 /* The option requires an argument. */
#define optional_argument 2 /* The option's argument is optional. */

#ifdef __cplusplus
extern "C"
{
#endif

    extern char *optarg;
    extern int optind, opterr, optopt, optreset;

    int getopt(int argc, char *const argv[], const char *optstring);

    struct option
    {
        const char *name;
        int has_arg;
        int *flag;
        int val;
    };

    int getopt_long(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx);
    int getopt_long_only(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
