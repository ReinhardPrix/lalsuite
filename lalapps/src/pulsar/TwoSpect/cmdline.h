/** @file cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.4
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef CMDLINE_H
#define CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "lalapps_TwoSpect"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "lalapps_TwoSpect"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "1.0.5"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *full_help_help; /**< @brief Print help, including hidden options, and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * config_arg;	/**< @brief Configuration file in gengetopt format for passing parameters.  */
  char * config_orig;	/**< @brief Configuration file in gengetopt format for passing parameters original value given at command line.  */
  const char *config_help; /**< @brief Configuration file in gengetopt format for passing parameters help description.  */
  int verbosity_arg;	/**< @brief Verbosity level (default='0').  */
  char * verbosity_orig;	/**< @brief Verbosity level original value given at command line.  */
  const char *verbosity_help; /**< @brief Verbosity level help description.  */
  double Tobs_arg;	/**< @brief Total observation time.  */
  char * Tobs_orig;	/**< @brief Total observation time original value given at command line.  */
  const char *Tobs_help; /**< @brief Total observation time help description.  */
  double Tcoh_arg;	/**< @brief SFT coherence time (default='1800').  */
  char * Tcoh_orig;	/**< @brief SFT coherence time original value given at command line.  */
  const char *Tcoh_help; /**< @brief SFT coherence time help description.  */
  double t0_arg;	/**< @brief Start time of the search in GPS seconds.  */
  char * t0_orig;	/**< @brief Start time of the search in GPS seconds original value given at command line.  */
  const char *t0_help; /**< @brief Start time of the search in GPS seconds help description.  */
  double fmin_arg;	/**< @brief Minimum frequency of band.  */
  char * fmin_orig;	/**< @brief Minimum frequency of band original value given at command line.  */
  const char *fmin_help; /**< @brief Minimum frequency of band help description.  */
  double fspan_arg;	/**< @brief Frequency span of band.  */
  char * fspan_orig;	/**< @brief Frequency span of band original value given at command line.  */
  const char *fspan_help; /**< @brief Frequency span of band help description.  */
  double Pmin_arg;	/**< @brief Minimum period to be searched (default='7200.0').  */
  char * Pmin_orig;	/**< @brief Minimum period to be searched original value given at command line.  */
  const char *Pmin_help; /**< @brief Minimum period to be searched help description.  */
  double Pmax_arg;	/**< @brief Maximum period to be searched.  */
  char * Pmax_orig;	/**< @brief Maximum period to be searched original value given at command line.  */
  const char *Pmax_help; /**< @brief Maximum period to be searched help description.  */
  double dfmin_arg;	/**< @brief Minimum modulation depth to search.  */
  char * dfmin_orig;	/**< @brief Minimum modulation depth to search original value given at command line.  */
  const char *dfmin_help; /**< @brief Minimum modulation depth to search help description.  */
  double dfmax_arg;	/**< @brief Maximum modulation depth to search.  */
  char * dfmax_orig;	/**< @brief Maximum modulation depth to search original value given at command line.  */
  const char *dfmax_help; /**< @brief Maximum modulation depth to search help description.  */
  char * IFO_arg;	/**< @brief Interferometer of whose data is being analyzed (default='H1').  */
  char * IFO_orig;	/**< @brief Interferometer of whose data is being analyzed original value given at command line.  */
  const char *IFO_help; /**< @brief Interferometer of whose data is being analyzed help description.  */
  double ihsfar_arg;	/**< @brief IHS FAR threshold (default='0.01').  */
  char * ihsfar_orig;	/**< @brief IHS FAR threshold original value given at command line.  */
  const char *ihsfar_help; /**< @brief IHS FAR threshold help description.  */
  double ihsfom_arg;	/**< @brief IHS FOM = 12*(L_IHS_loc - U_IHS_loc)^2.  */
  char * ihsfom_orig;	/**< @brief IHS FOM = 12*(L_IHS_loc - U_IHS_loc)^2 original value given at command line.  */
  const char *ihsfom_help; /**< @brief IHS FOM = 12*(L_IHS_loc - U_IHS_loc)^2 help description.  */
  double ihsfomfar_arg;	/**< @brief IHS FOM FAR threshold.  */
  char * ihsfomfar_orig;	/**< @brief IHS FOM FAR threshold original value given at command line.  */
  const char *ihsfomfar_help; /**< @brief IHS FOM FAR threshold help description.  */
  double tmplfar_arg;	/**< @brief Template FAR threshold (default='0.01').  */
  char * tmplfar_orig;	/**< @brief Template FAR threshold original value given at command line.  */
  const char *tmplfar_help; /**< @brief Template FAR threshold help description.  */
  double avesqrtSh_arg;	/**< @brief Expected average of square root of Sh (default='1.0').  */
  char * avesqrtSh_orig;	/**< @brief Expected average of square root of Sh original value given at command line.  */
  const char *avesqrtSh_help; /**< @brief Expected average of square root of Sh help description.  */
  int blksize_arg;	/**< @brief Blocksize for running median of 1st FFT band (default='1001').  */
  char * blksize_orig;	/**< @brief Blocksize for running median of 1st FFT band original value given at command line.  */
  const char *blksize_help; /**< @brief Blocksize for running median of 1st FFT band help description.  */
  char * outdirectory_arg;	/**< @brief Output directory.  */
  char * outdirectory_orig;	/**< @brief Output directory original value given at command line.  */
  const char *outdirectory_help; /**< @brief Output directory help description.  */
  char * sftDir_arg;	/**< @brief Directory containing SFTs (default='./').  */
  char * sftDir_orig;	/**< @brief Directory containing SFTs original value given at command line.  */
  const char *sftDir_help; /**< @brief Directory containing SFTs help description.  */
  char * ephemDir_arg;	/**< @brief Path to ephemeris files (default='/opt/lscsoft/lalpulsar/share/lalpulsar').  */
  char * ephemDir_orig;	/**< @brief Path to ephemeris files original value given at command line.  */
  const char *ephemDir_help; /**< @brief Path to ephemeris files help description.  */
  char * ephemYear_arg;	/**< @brief Year or year range (e.g. 08-11) of ephemeris files (default='08-11').  */
  char * ephemYear_orig;	/**< @brief Year or year range (e.g. 08-11) of ephemeris files original value given at command line.  */
  const char *ephemYear_help; /**< @brief Year or year range (e.g. 08-11) of ephemeris files help description.  */
  double dopplerMultiplier_arg;	/**< @brief Multiplier for the Doppler velocity (default='1.0').  */
  char * dopplerMultiplier_orig;	/**< @brief Multiplier for the Doppler velocity original value given at command line.  */
  const char *dopplerMultiplier_help; /**< @brief Multiplier for the Doppler velocity help description.  */
  int templateLength_arg;	/**< @brief Maximum number of pixels to use in the template (default='50').  */
  char * templateLength_orig;	/**< @brief Maximum number of pixels to use in the template original value given at command line.  */
  const char *templateLength_help; /**< @brief Maximum number of pixels to use in the template help description.  */
  char * skyRegion_arg;	/**< @brief Region of the sky to search (e.g. (ra1,dec1),(ra2,dec2),(ra3,dec3)...) or allsky (default='allsky').  */
  char * skyRegion_orig;	/**< @brief Region of the sky to search (e.g. (ra1,dec1),(ra2,dec2),(ra3,dec3)...) or allsky original value given at command line.  */
  const char *skyRegion_help; /**< @brief Region of the sky to search (e.g. (ra1,dec1),(ra2,dec2),(ra3,dec3)...) or allsky help description.  */
  double SFToverlap_arg;	/**< @brief SFT overlap in seconds, usually Tcoh/2 (default='900').  */
  char * SFToverlap_orig;	/**< @brief SFT overlap in seconds, usually Tcoh/2 original value given at command line.  */
  const char *SFToverlap_help; /**< @brief SFT overlap in seconds, usually Tcoh/2 help description.  */
  char * sftType_arg;	/**< @brief Expected SFT from either 'MFD' (Makefakedata_v4) or 'vladimir' (Vladimir's SFT windowed version) which uses a factor of 2 rather than sqrt(8/3) for the window normalization (default='vladimir').  */
  char * sftType_orig;	/**< @brief Expected SFT from either 'MFD' (Makefakedata_v4) or 'vladimir' (Vladimir's SFT windowed version) which uses a factor of 2 rather than sqrt(8/3) for the window normalization original value given at command line.  */
  const char *sftType_help; /**< @brief Expected SFT from either 'MFD' (Makefakedata_v4) or 'vladimir' (Vladimir's SFT windowed version) which uses a factor of 2 rather than sqrt(8/3) for the window normalization help description.  */
  int markBadSFTs_flag;	/**< @brief Mark bad SFTs (default=off).  */
  const char *markBadSFTs_help; /**< @brief Mark bad SFTs help description.  */
  int FFTplanFlag_arg;	/**< @brief 0=Estimate, 1=Measure, 2=Patient, 3=Exhaustive (default='3').  */
  char * FFTplanFlag_orig;	/**< @brief 0=Estimate, 1=Measure, 2=Patient, 3=Exhaustive original value given at command line.  */
  const char *FFTplanFlag_help; /**< @brief 0=Estimate, 1=Measure, 2=Patient, 3=Exhaustive help description.  */
  int IHSonly_flag;	/**< @brief IHS stage only is run. Output statistic is the IHS statistic. (default=off).  */
  const char *IHSonly_help; /**< @brief IHS stage only is run. Output statistic is the IHS statistic. help description.  */
  int BrentsMethod_flag;	/**< @brief Use Brent's method in the root finding algorithm. (default=off).  */
  const char *BrentsMethod_help; /**< @brief Use Brent's method in the root finding algorithm. help description.  */
  int antennaOff_flag;	/**< @brief Antenna pattern weights are /NOT/ used if this flag is used (default=off).  */
  const char *antennaOff_help; /**< @brief Antenna pattern weights are /NOT/ used if this flag is used help description.  */
  int noiseWeightOff_flag;	/**< @brief Turn off noise weighting if this flag is used (default=off).  */
  const char *noiseWeightOff_help; /**< @brief Turn off noise weighting if this flag is used help description.  */
  int gaussTemplatesOnly_flag;	/**< @brief Gaussian templates only throughout the pipeline if this flag is used (default=off).  */
  const char *gaussTemplatesOnly_help; /**< @brief Gaussian templates only throughout the pipeline if this flag is used help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int full_help_given ;	/**< @brief Whether full-help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int config_given ;	/**< @brief Whether config was given.  */
  unsigned int verbosity_given ;	/**< @brief Whether verbosity was given.  */
  unsigned int Tobs_given ;	/**< @brief Whether Tobs was given.  */
  unsigned int Tcoh_given ;	/**< @brief Whether Tcoh was given.  */
  unsigned int t0_given ;	/**< @brief Whether t0 was given.  */
  unsigned int fmin_given ;	/**< @brief Whether fmin was given.  */
  unsigned int fspan_given ;	/**< @brief Whether fspan was given.  */
  unsigned int Pmin_given ;	/**< @brief Whether Pmin was given.  */
  unsigned int Pmax_given ;	/**< @brief Whether Pmax was given.  */
  unsigned int dfmin_given ;	/**< @brief Whether dfmin was given.  */
  unsigned int dfmax_given ;	/**< @brief Whether dfmax was given.  */
  unsigned int IFO_given ;	/**< @brief Whether IFO was given.  */
  unsigned int ihsfar_given ;	/**< @brief Whether ihsfar was given.  */
  unsigned int ihsfom_given ;	/**< @brief Whether ihsfom was given.  */
  unsigned int ihsfomfar_given ;	/**< @brief Whether ihsfomfar was given.  */
  unsigned int tmplfar_given ;	/**< @brief Whether tmplfar was given.  */
  unsigned int avesqrtSh_given ;	/**< @brief Whether avesqrtSh was given.  */
  unsigned int blksize_given ;	/**< @brief Whether blksize was given.  */
  unsigned int outdirectory_given ;	/**< @brief Whether outdirectory was given.  */
  unsigned int sftDir_given ;	/**< @brief Whether sftDir was given.  */
  unsigned int ephemDir_given ;	/**< @brief Whether ephemDir was given.  */
  unsigned int ephemYear_given ;	/**< @brief Whether ephemYear was given.  */
  unsigned int dopplerMultiplier_given ;	/**< @brief Whether dopplerMultiplier was given.  */
  unsigned int templateLength_given ;	/**< @brief Whether templateLength was given.  */
  unsigned int skyRegion_given ;	/**< @brief Whether skyRegion was given.  */
  unsigned int SFToverlap_given ;	/**< @brief Whether SFToverlap was given.  */
  unsigned int sftType_given ;	/**< @brief Whether sftType was given.  */
  unsigned int markBadSFTs_given ;	/**< @brief Whether markBadSFTs was given.  */
  unsigned int FFTplanFlag_given ;	/**< @brief Whether FFTplanFlag was given.  */
  unsigned int IHSonly_given ;	/**< @brief Whether IHSonly was given.  */
  unsigned int BrentsMethod_given ;	/**< @brief Whether BrentsMethod was given.  */
  unsigned int antennaOff_given ;	/**< @brief Whether antennaOff was given.  */
  unsigned int noiseWeightOff_given ;	/**< @brief Whether noiseWeightOff was given.  */
  unsigned int gaussTemplatesOnly_given ;	/**< @brief Whether gaussTemplatesOnly was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];
/** @brief all the lines making the full help output (including hidden options) */
extern const char *gengetopt_args_info_full_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the full help (including hidden options)
 */
void cmdline_parser_print_full_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * The config file parser (deprecated version)
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_config_file() instead
 */
int cmdline_parser_configfile (const char *filename,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The config file parser
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_config_file (const char *filename,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * The string parser (interprets the passed string as a command line)
 * @param cmdline the command line stirng
 * @param args_info the structure where option information will be stored
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_string (const char *cmdline, struct gengetopt_args_info *args_info,
  const char *prog_name);
/**
 * The string parser (version with additional parameters - deprecated)
 * @param cmdline the command line stirng
 * @param args_info the structure where option information will be stored
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_string_ext() instead
 */
int cmdline_parser_string2 (const char *cmdline, struct gengetopt_args_info *args_info,
  const char *prog_name,
  int override, int initialize, int check_required);
/**
 * The string parser (version with additional parameters)
 * @param cmdline the command line stirng
 * @param args_info the structure where option information will be stored
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_string_ext (const char *cmdline, struct gengetopt_args_info *args_info,
  const char *prog_name,
  struct cmdline_parser_params *params);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMDLINE_H */
