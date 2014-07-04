
/***********************************************************************

        Copyright (C) 1991,
        Virginia Polytechnic Institute & State University

        This program was originally written by Mr. Hyung K. Lee
        under the supervision of Dr. Dong S. Ha, in the Bradley
        Department of Electrical Engineering, VPI&SU, in 1991.

        This program is released for research use only. This program,
        or any derivative thereof, may not be reproduced nor used
        for any commercial product without the written permission
        of the authors.

        For detailed information, please contact to

        Dr. Dong S. Ha
        Bradley Department of Electrical Engineering
        Virginia Polytechnic Institute & State University
        Blacksburg, VA 24061

        Ph.: (540) 231-4942
        Fax: (540) 231-3362
        E-Mail: ha@vt.edu
        Web: http://www.ee.vt.edu/ha

        REFERENCE:
           H. K. Lee and D. S. Ha, "On the Generation of Test Patterns
           for Combinational Circuits," Technical Report No. 12_93,
           Dep't of Electrical Eng., Virginia Polytechnic Institute
           and State University.

***********************************************************************/

/**************************** HISTORY **********************************
 
        atalanta: version 1.0        H. K. Lee, 8/15/1991
        atalanta: version 1.1        H. K. Lee, 10/5/1992
        atalanta: version 2.0        H. K. Lee, 6/30/1997
 
***********************************************************************/

/*----------------------------------------------------------------------
	global.d
	Define global variables of atalanta.
-----------------------------------------------------------------------*/

#ifndef __ATALANTA_GLOBAL_D__
#define __ATALANTA_GLOBAL_D__

#include "define.h"

// my global definitions
   extern char inputmode;
   extern char rptmode;
   extern char logmode;
   extern char cctmode;
   extern int LEVEL;
   extern GATEPTR *m_stem;
   extern int m_nstem;
   extern int nredundant;
   extern int maxdetect;
   extern int m_ndetect;
   extern int m_maxbits;
   extern int m_ntest;
   extern int m_npacket;
   extern int m_nbit;
   extern int phase2;
   extern double minutes,seconds,starttime,inittime,runtime1,runtime2;
   extern double fantime,fan1time,simtime1,simtime2,simtime3;
   extern int noverbacktrack;
   extern int tbacktrack;
   extern int ntest2, ntest3;
   extern int shuf;

   extern TestVectors tv;             // test vectors will be stored here
   extern FAULTTYPE *my_cur_fault;         // current fault processed
   extern char * pattern_file;
   extern char * bench_file;
   extern char * fault_file;
   extern char * w_fault_file;
   extern char * UD_faults_file;
   extern char * mask_file;
   extern char * report_file;
   extern char LFSR_poly[1000];
   extern char LFSR_seed[1000];
   extern int LFSR_num;
   extern int LFSR_sim_mode; 

   /*For Transition Fault*/
   extern char output_file[100];
   extern char * TF_line;
   extern char * TF_type;
   extern char pattern1[MAXPI + 1];
   extern char pattern2[MAXPI + 1];
   /*For Transition Fault*/

extern GATEPTR *net;		/* circuit structure */
extern int *primaryin,*primaryout,*flip_flops,*headlines;
extern int nog,nopi,nopo,noff,nof,nodummy,lastgate;
extern int maxlevel,POlevel,PPOlevel;

extern int *depth_array;
extern STACKPTR event_list;		/* event list */
extern struct FAULT **faultlist;	/* fault list */

#ifdef INCLUDE_HOPE
extern FAULTPTR headfault,tailfault,currentfault;
extern EVENTPTR head,tail;
#endif

extern struct ROOTTREE tree;

/* static buffers for fan */
extern STACKTYPE unjustified,		/* set of unjustified lines */
	  init_obj,		/* set of initial objectives */
	  curr_obj,		/* set of current objectives */
	  fan_obj,		/* set of fanout objectives */
	  head_obj,		/* set of head objectives */
	  final_obj,		/* set of final objectives */
	  Dfrontier,		/* set of Dfrotiers */
	  stack;		/* stack for backtracing */

/* buffers for the fault simulator */
extern STACKTYPE free_gates,		/* fault free simulation */
	  faulty_gates,		/* list of faulty gates */
	  eval_gates,		/* STEM_LIST to be simulated */
	  active_stems;		/* list of active stems */
extern GATEPTR *dynamic_stack;
extern int nsstack,ndstack;

extern FILE *circuit,*test,*logfile;
extern int mac_i;

/* global variables for bit operations */
extern level all_one;
extern status update_flag, update_flag2;
extern struct STACK stack1, stack2;

/* Variables for hope */
extern char initialmode;
extern char xdetectmode;

extern level InVal[MAXPI];

extern level BITMASK[32];

extern char *fn_to_string[MAXGTYPE+3];
extern char *level_to_string[MAXLEVEL+1];
extern char *fault_to_string[2];   /* fault type to string */
extern level parallel_to_level[2][2];

#endif /* __ATALANTA_GLOBAL_D__ */
