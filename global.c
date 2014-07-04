
#include "define.h"
#include <stdio.h>

// my global definitions
   char inputmode='d';
   char rptmode='y';
   char logmode='n';
   char cctmode=ISCAS89;
   int LEVEL=0;
   GATEPTR *m_stem;
   int m_nstem;
   int nredundant=0;
   int maxdetect;
   int m_ndetect=0;
   int m_maxbits=BITSIZE;
   int m_ntest=0;
   int m_npacket=0;
   int m_nbit=0;
   int phase2;
   double minutes,seconds,starttime,inittime,runtime1,runtime2;
   double fantime,fan1time,simtime1,simtime2,simtime3;
   int noverbacktrack=0;
   int tbacktrack=0;
   int ntest2=0, ntest3=0;
   int shuf;

   TestVectors tv;             // test vectors will be stored here
   FAULTTYPE *my_cur_fault;         // current fault processed
   char * pattern_file = NULL;
   char * bench_file = NULL;
   char * fault_file = NULL;
   char * w_fault_file = NULL;
   char * UD_faults_file = NULL;
   char * mask_file = NULL;
   /*For Transition Fault*/
   char  output_file[100];		//Output file name (DUT_target_fault.txt)
   char * TF_line = NULL;		//Transition fault target line
   char * TF_type = NULL;		//Transition fault type (STR or STF)
   char  pattern1[MAXPI + 1];	//Temperary pattern file 1's name
   char  pattern2[MAXPI + 1];	//Temperary pattern file 2's name
   /*For Transition Fault END*/
   char * report_file = NULL;
   char LFSR_poly[1000];
   char LFSR_seed[1000];
   int LFSR_num;
   int LFSR_sim_mode = 0;



GATEPTR *net;		/* circuit structure */
int *primaryin,*primaryout,*flip_flops,*headlines;
int nog=0,nopi=0,nopo=0,noff=0,nof=0,nodummy=0,lastgate;
int maxlevel,POlevel,PPOlevel;

int *depth_array;
STACKPTR event_list;		/* event list */
struct FAULT **faultlist;	/* fault list */

#ifdef INCLUDE_HOPE
FAULTPTR headfault,tailfault,currentfault;
EVENTPTR head,tail;
#endif

struct ROOTTREE tree;

/* static buffers for fan */
STACKTYPE unjustified,		/* set of unjustified lines */
	  init_obj,		/* set of initial objectives */
	  curr_obj,		/* set of current objectives */
	  fan_obj,		/* set of fanout objectives */
	  head_obj,		/* set of head objectives */
	  final_obj,		/* set of final objectives */
	  Dfrontier,		/* set of Dfrotiers */
	  stack;		/* stack for backtracing */

/* buffers for the fault simulator */
STACKTYPE free_gates,		/* fault free simulation */
	  faulty_gates,		/* list of faulty gates */
	  eval_gates,		/* STEM_LIST to be simulated */
	  active_stems;		/* list of active stems */
GATEPTR *dynamic_stack;
int nsstack,ndstack;

FILE *circuit,*test,*logfile;
int mac_i;

/* global variables for bit operations */
level all_one;
status update_flag, update_flag2;
struct STACK stack1, stack2;

/* Variables for hope */
char initialmode='x';
char xdetectmode='n';

level InVal[MAXPI];

level BITMASK[32]=
        { MASK0, MASK0<<1, MASK0<<2, MASK0<<3,
          MASK0<<4, MASK0<<5, MASK0<<6, MASK0<<7,
          MASK0<<8, MASK0<<9, MASK0<<10, MASK0<<11,
          MASK0<<12, MASK0<<13, MASK0<<14, MASK0<<15,
          MASK0<<16, MASK0<<17, MASK0<<18, MASK0<<19,
          MASK0<<20, MASK0<<21, MASK0<<22, MASK0<<23,
          MASK0<<24, MASK0<<25, MASK0<<26, MASK0<<27,
          MASK0<<28, MASK0<<29, MASK0<<30, MASK0<<31
        };

char *fn_to_string[MAXGTYPE+3]=         /* gate function to string */
{"AND","NAND","OR","NOR","INPUT","XOR","XNOR","DFF","DUMMY","BUFFER","NOT",
 "","","","","","","","","","PO",};
char *level_to_string[MAXLEVEL+1]=      /* level to string */
{"0","1","x","z",};
char *fault_to_string[2]={"/0","/1"};   /* fault type to string */
level parallel_to_level[2][2]=          /* parallel level types to level */
{{X,ONE},{ZERO,Z}};

