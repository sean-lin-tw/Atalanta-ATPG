/*---------------------------------------------------------------------
	atalanta.c
	Main program for atalanta.
	An automatic test pattern generator for single stuck-at
	fault in combinational logic circuits.
	Generates test patterns detecting stuck-at faults in
	combinational circuits.
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>

#include "parameter.h"
#include "define.h"
#include "macro.h"
#include "truthtable.h"

#include "global.d"

#define CHECKPOINTMODE 1
#define DEFAULTMODE 0

/*extern caddr_t sbrk();*/
extern long random(void);
extern int  init_fault();
extern void print_atpg_head(FILE *fp), print_atpg_result(FILE *fp, char *name, int ng, int npi, int npo, int mlev, int mback, int mback1, int ph2, int nt2, int nt3, int nof, int nd, int nred, int tback, int nshuf, float inittime, float simtime, float fantime, float runtime, char mode, int memory),
	    exit(int), fatalerror(int errorcode), learn(int nog, int maxdpi);
//extern int print_undetected_faults(FILE *fp, char symbol, char rfaultmode, int flag);

#define valid_test(i) test_vectors[i][nopi]=ONE
#define invalid_test(i) test_vectors[i][nopi]=ZERO
#define is_valid(i) (test_vectors[i][nopi]==ONE)
#define is_invalid(i) (test_vectors[i][nopi]==ZERO)
#define is_random_mode(mode) (mode=='y')
#define delete_fault(fal) \
if(fal->previous==fal) { \
   fal->gate->pfault=fal->next; \
   if(fal->next!=NULL) fal->next->previous=fal->next; \
} else { \
   fal->previous->next=fal->next; \
   if(fal->next!=NULL) fal->next->previous=fal->previous; \
}

#define is_checkpoint(gate) (gate->fn>=PI || gate->noutput>1)
#define output0 output

#define checkbit(word,nth) ((word&BITMASK[nth])!=ALL0)
#define setbit(word,nth) (word|=BITMASK[nth])
#define resetbit(word,nth) (word&=(~BITMASK[nth]))

/* external variables */
extern void setfanoutstem(int nog, GATEPTR *stem, int nstem);
extern void set_unique_path(int nog, int maxdpi);
extern void pinit_simulation(int nog, int maxdpi, int npi);
extern void GetPRandompattern(register int number, level *array);
extern void pfault_free_simulation(void);
extern void update_all1(int npi);
extern char *strcpy(char *, const char *), *strcat(char *, const char *);
extern void GetRandompattern(register int number, level *array, int nbit);
extern void print_log_topic(FILE *fp, char *name);
extern void gettime(double *usertime, double *systemtime, double *total);
//extern void printinputs(FILE *fp, int npi, int nth_bit), printoutputs(FILE *fp, int npo, int nth_bit), printfault(FILE *fp, FAULTPTR f, boolean mode);
extern void set_testability(int nog);
extern int Simulate_Hope(int nopi, int nopo, int *npacket, int *nbit);

/* variables for main program */
level test_vectors[MAXTEST/10][MAXPI+1];
level test_vectors1[MAXTEST/10][MAXPI+1];
level test_store[MAXTEST/10][MAXPI+1];
level test_store1[MAXTEST/10][MAXPI+1];


/* default parameters setting */
char cctname[100]="";
char fillmode='r';		/* 0, 1, x, r */
char compact='s';		/* n: no compaction, r: reverse, s: shuffle */
char learnmode='n';
int iseed=0;			/* initial random seed */
int randomlimit=16;		/* condition for RPT stopping */
int maxbacktrack=10;		/* maximum backtracking of FAN */
int maxbacktrack1=0;		/* maximum backtracking of FAN */
int maxcompact=2;		/* maximum limit for compaction*/
char _MODE_SIM='f';		/* 'f': FSIM, 'h': HOPE */
char faultmode='d';
char gen_all_pat='n';		/* 'y': generates all patterns for each fault */
int ntest_each_limit=(-1);      /* no of patterns to be generated for each fault */
char no_faultsim='n';		/* 'y': no fault simulation */
//char rfaultmode='n';            /* Write out redundant faults */

FILE *faultfile;
FILE *ufaultfile;
int msize;
int w_faults = 0;
int ufaultmode = 0;            /* Write out undetected faults */
int simulation_mode = 0;
int w_test_mode = 0;

/*For Transition Fault */
int TFmode = 0;
/*For Transition fault END*/


/*------main: Main program of atalanta---------------------------*/


int option_set (int argc, char **argv);

//random.c
extern int Seed (int startvalue);

//read_cct.c
extern int read_circuit (FILE *circuit, char *name);

//structure.c
extern int add_PO (void);
extern void allocate_stacks (void);
extern int compute_level (void);
extern void allocate_event_list (void);
extern int levelize (void);
extern void add_sparegates (void);

//stem.c
extern int SetFFR (void);
extern int SetDominator (void);

//io.c
extern int set_cct_parameters (int nog, int npi);
extern boolean allocate_dynamic_buffers (int nog);
extern int set_dominator (int nog, int maxdpi);

//define_fault_list.c
extern int readfaults (FILE *file, int nog, int no_stem, GATEPTR *stem);
extern int set_all_fault_list (int nog, int no_stem, GATEPTR *stem);
extern void FWD_faults (void);
extern void readfaults_hope (FILE *file);
extern int check_redundant_faults (int nog);
/*For transition fault*/
extern int set_all_UC_fault_list(int nog, int no_stem, GATEPTR *stem);
/*For transition fault END*/
//extern boolean print_test_topic (FILE *fp, int npi, int npo, char *name);

//fim.c
extern void InitFaultSim (void);

//sim.c
extern int random_sim (int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, level *LFSR, int limit, int maxbit, int maxdetect, int *ntest, int *npacket, int *nbit);
extern int testgen (int nog, int nopi, int nopo, int LEVEL, int maxbits, int nstem, GATEPTR *stem, int maxbacktrack, int phase, int *nredundant, int *noverbacktrack, int *nbacktrack, int *ntest, int *npacket, int *nbit, double *fantime);
extern int compact_test (int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, int nof, int *nshuf, int *ndet, int npacket, int nbit, int maxbits);

/*For transition fault*/
void WriteTFList();
//撠arget TF頧??拙AF?ault list瑼?
void SetMode(int TFstatus);
//閮剖?ATPG?ault Simulation????
int	 FileEmpty(char * name);
//瑼Ｘ?臬?箇征瑼?
void New_MakeFaultList(my_FaultList * fl);
//撱箇?my_FaultList鞈?蝯?
void ReadPattern(char * name, int i);
//霈?TPG?Ｙ??attern
void SetPatFile(char * name);
//?杗TPG?Ｙ??attern銝剝???蒂摮Pattern File
void GetcctName();
//霈?頝臬?蝔?
void WriteTFReport(my_FaultList * tfl_1, my_FaultList * tfl_2);
//撠??神?叨utput瑼?
int  CompareFaults(char * name1, char * name2);
//瘥?Fault List,?曉??嗡?Transition Fault????
/*For transition fault END*/
// --------------------------------------------------------------

void InitFS();
void SetAllFaults();
void SetFaults();


ATPG_Results GetResults()
{
    ATPG_Results ar;

    strcpy( ar.cctname, cctname );
    ar.gates = nog - nopi - nopo;
    ar.iv = nopi;
    ar.ov = nopo;
    ar.i_patterns = ntest2;
    ar.patterns = ntest3;
    ar.faults = nof;
    ar.d_faults = m_ndetect;
    ar.r_faults = nredundant;
    return ar;
}

void IndexFaults()
{
    int i;

    for ( i = 0; i < nof; i++ )
        faultlist[i]->index = i;
}

extern char *fault2str[4];
//char *fault2str[4]={" /0"," /1"," /0"," /1"};
my_FaultList * MakeFaultList()
{
    my_FaultList * fl;
    int i;
    fl = malloc( sizeof(my_FaultList) );
    fl->faults = nof;
    fl->mask = malloc( nof );
    memset( fl->mask, nof, 0 );
    fl->names = malloc( sizeof(char*) * nof );
    for ( i = 0; i < nof; i++ ) {
        fl->names[i] = malloc( 100 );
        fl->names[i][0] = 0;
        if ( faultlist[i]->line >= 0 ) {
            strcpy( fl->names[i], faultlist[i]->gate->inlis[faultlist[i]->line]->symbol->symbol );
            strcat( fl->names[i], "->" );
        }
        strcat( fl->names[i], faultlist[i]->gate->symbol->symbol );
        strcat( fl->names[i], fault2str[faultlist[i]->type] );
    }
	return fl;
}

void UpdateFaultList( my_FaultList * fl )
{
    int i;

    for ( i = 0; i < nof; i++ )
	{
		fl->mask[i] = faultlist[i]->detected;	
	}
}


void AddTestVector( char * ivct, char * ovct, int no )
/*
    Adds a test vector. According "my_cur_fault" sets the struct (for -D n)
*/

{
    TestVectorType * testv;

    testv = malloc(sizeof(TestVectorType));
    testv->ivct = malloc( tv.inpvars + 1 );
    testv->mask = NULL;
    strcpy(testv->ivct, ivct);
    if ( ovct != NULL ) {
        testv->ovct = malloc( tv.outvars + 1 );
        strcpy(testv->ovct, ovct);
    } else testv->ovct = NULL;
    if ( my_cur_fault != NULL ) {
        if(my_cur_fault->line >= 0)
             testv->flt_line_hash = my_cur_fault->gate->inlis[my_cur_fault->line]->symbol->key;
        else testv->flt_line_hash = -1;
        testv->flt_hash = my_cur_fault->gate->symbol->key;
        testv->type = my_cur_fault->type % 2;
        testv->index = my_cur_fault->index;
    } else {
        testv->flt_line_hash = -1;
        testv->flt_hash = -1;
        testv->type = -1;
        testv->index = -1;
    }

    testv->no = no;
    testv->next = tv.vcts;
    tv.vcts = testv;
    tv.num++;
}


int SimulateVector( char * vct )
/*
    Simulates one vector by HOPE, returns # of faults detected
*/
{

	int i;

    for (i = 0; i < nopi; i++)
        switch ( vct[i] ) {
            case '0': InVal[i] = ZERO;
                      break;
            case '1': InVal[i] = ONE;
                      break;
            case 'x':
            case 'X':
            case '-':
            case '2': InVal[i] = X;
                      break;
            default:  InVal[i] = X;
                      break;
        }
    return Simulate_Hope( nopi, nopo, &m_npacket, &m_nbit);
}

int SimulateTest()
/*
    Simulates whole test in "tv" by HOPE, returns # of faults detected
*/
{
   TestVectorType * testv;
   int det;

   det = 0;
   testv = tv.vcts;
   while ( testv != NULL ) {
       det += SimulateVector( testv->ivct );
       testv = testv->next;
   }
   m_ndetect = det;
   return det;
}

void SimulateAllVectors()
/*
    Simulates all the test vectors in "tv" and generates the "tv.mask" structure
*/
{
   TestVectorType * testv;
   int i;
   int det;

   testv = tv.vcts;
   det = 0;
   SetFaults();
   InitFS();
   while ( testv != NULL ) {
       for(i=0;i<nof;i++) {
          faultlist[i]->detected=UNDETECTED;
          faultlist[i]->observe=ALL0;
       }
       det = SimulateVector( testv->ivct );
       free(testv->mask);
       testv->mask = malloc( nof );
       for ( i = 0; i < nof; i++ )
          testv->mask[i] = faultlist[i]->detected;
       testv = testv->next;
   }
}


char * OctToBin( char *c )
{
    int i, p, n, lead;
    char * num;
    
    num = malloc( nopi + 2 );
    memset( num, nopi, '0' );
    p = 0;
    lead = 1;	
    for (i = 0; i < strlen(c); i++) {
    	n = c[i]-'0';
        if (n > 3) {
        	num[p++] = '1';
        	lead = 0;
        	n -= 4;
        } else if (lead == 0) num[p++] = '0';
        if (n > 1) {
        	num[p++] = '1';
        	lead = 0;
        	n -= 2;
        } else if (lead == 0) num[p++] = '0';
        if (n > 0) {
        	num[p++] = '1';
        	lead = 0;
        } else if (lead == 0) num[p++] = '0';
    }
    num[p] = 0;
    return num;
}

int intxor(int a, int b)
{
    if (a == b) return 0; else return 1;
}

int SimulateLFSR()
{
    int i, j, t;
    int det;
    char * poly;
    char * state;
    int order;

    poly = OctToBin( LFSR_poly );
    state = OctToBin ( LFSR_seed );
    det = 0;
    order = strlen(poly) - 1;

    for ( i = 0; i < LFSR_num; i++ ) {
        det += SimulateVector( state );

        t = state[order-1] - '0';
        for ( j = order-1; j > 0; j-- )
            state[j] = intxor( state[j-1]-'0', t*(poly[j]-'0')) + '0';
        state[0] = t + '0';
    }
    m_ndetect = det;
    return det;
}

void ReadBenchFile( char *name )
{
   int i, j;


   if ( name != NULL ) {

       if((circuit = fopen(name,"rt")) == NULL) {
          fprintf(stderr,"Fatal error: no such file exists %s\n", name);
          exit(0);
       }
       if(read_circuit(circuit,cctname) < 0) fatalerror(CIRCUITERROR);
       fclose(circuit);

       if(nog<=0 || nopi<=0 || nopo<=0) {
          fprintf(stderr,"Error: #pi=%d, #po=%d, #gate=%d\n",nopi,nopo,nog);
          fatalerror(CIRCUITERROR);
       }
       if(noff > 0) {
          fprintf(stderr,"Error: %d flip-flop exists in the circuit.\n",noff);
          fatalerror(CIRCUITERROR);
       }
       nodummy=add_PO();

       if(cctmode==ISCAS89) {
          ALLOCATE(stack.list,GATEPTR,nog+10);
          clear(stack);

    #ifdef INCLUDE_HOPE
          allocate_stacks();
          maxlevel=compute_level();
          allocate_event_list();
          levelize();
          add_sparegates();
          lastgate=nog-1;

          i=SetFFR();
          j=SetDominator();
    #else
          if(levelize(net,nog,nopi,nopo,noff,stack.list) <0) {
             fprintf(stderr,"Fatal error: Invalid circuit file.\n");
             exit(0);
          }
    #endif

          if(noff > 0) {
             fprintf(stderr,"Error: Invalid type DFF is defined.\n");
             exit(0);
          }
       } else stack.list=NULL;

       maxlevel=set_cct_parameters(nog,nopi);

    #ifdef INCLUDE_HOPE
       LEVEL=maxlevel+2;
    #else
       LEVEL=maxlevel;
    #endif

       if(_MODE_SIM=='f') {
          if(!allocate_dynamic_buffers(nog)) {
             fprintf(stderr,"Fatal error: memory allocation error\n");
             exit(0);
          }

          m_nstem=0;
          for(i=0;i<nog;i++)
             if(is_fanout(net[i]) || net[i]->fn==PO) m_nstem++;
          m_stem=(GATEPTR *)malloc((unsigned)(sizeof(GATEPTR)*m_nstem));
          setfanoutstem(nog,m_stem,m_nstem);

       }
    }

}
/*
void OpenTestFile( char *name )

//    Opens test file for writing

{
   if((test = fopen(name, "wt")) == NULL) {
      fprintf(stderr,"Fatal error: %s file open error\n", name);
      exit(0);
   }
}
*/
/*
void OpenLogFile( char *name )

//    Opens log file for writing

{    if((logfile = fopen(name, "wt")) == NULL) {
      fprintf(stderr,"Fatal error: %s file open error\n", name);
      exit(0);
     }
} */

void ReadFaults( char *name )
{
   FILE *faultfile;

   if ( name != NULL ) {
      if((faultfile = fopen(name,"rt")) == NULL) {
        fprintf(stderr,"Fatal error: %s file open error\n",name);
        exit(0);
       }
      if(_MODE_SIM=='f')
        nof = readfaults(faultfile,nog,m_nstem,m_stem);

  #ifdef INCLUDE_HOPE
      else readfaults_hope(faultfile);
  #endif
      fclose(faultfile);

      if(nof<0) {
        fprintf(stderr,"Fatal error: error in setting fault list\n");
        exit(0);
      }
  }
}

void SetFaults()
{
   if (faultmode == 'f')
      ReadFaults( fault_file );
   else
      SetAllFaults();
}

void ReadTestFile( char * name )
/*
    Reads test file (pat), puts into the "tv" var.
*/

{
    FILE * f;
    int i;
    char s[MAXPI + 1];

    if ( name != NULL ) {
        tv.num = 0;
        tv.inpvars = nopi;
        tv.outvars = nopo;

        f = fopen( name, "rt" );
        while ( !feof( f )) {
            fscanf( f, "%s", s );
            my_cur_fault = NULL;
            if ( strlen( s ) != nopi ) {
                fprintf(stderr,"Fatal error: Incorrect number of test vector inputs\n");
                exit(0);
            }
            AddTestVector( s, NULL, -1 );
        }
        fclose(f);
    }
}

void SetAllFaults()
{
   if(_MODE_SIM=='f') { /* FSIM */
      nof = set_all_fault_list(nog,m_nstem,m_stem);
   }

#ifdef INCLUDE_HOPE
   else FWD_faults();
#endif

   if(nof<0) {
      fprintf(stderr,"Fatal error: error in setting fault list\n");
      exit(0);
   }
}

void InitFS()
{
   int i;
   
   set_testability(nog);

   for(i=0;i<nog;i++) {
      reset(net[i]->changed);
      net[i]->freach=nog;
      if(net[i]->dpi>=PPOlevel)
         printf("Error: gut=%s dpi=%d\n",net[i]->symbol->symbol, net[i]->dpi);

   }
   set_dominator(nog,LEVEL);
   set_unique_path(nog,LEVEL);

//   print_test_topic(test,nopi,nopo,name1);
   if(learnmode=='y') learn(nog,LEVEL);

   for(i=0;i<nof;i++) {
      faultlist[i]->detected=UNDETECTED;
      faultlist[i]->observe=ALL0;
   }

   if(_MODE_SIM=='f') {
      nredundant=check_redundant_faults(nog);
      pinit_simulation(nog,LEVEL,nopi);
   } else {
     InitFaultSim();
   }

   maxdetect=nof;

   all_one=ALL1;
}

void WriteTestFile( char * fn )
/*
    Writes a test file. Only test vectors (pat format).
    For -D n does not distinguish between vectors for the same fault - do not use here!
*/
{
    FILE * f;
    int i;
    TestVectorType * testv;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        testv = tv.vcts;
        while ( testv != NULL ) {
            fprintf( f, "%s\n", testv->ivct );
            testv = testv->next;
        }
        fclose(f);
    }
}

void WriteTestFile_out( char * fn )
/*
    Writes a test file. Test vectors with outputs.
    For -D n does not distinguish between vectors for the same fault - do not use here!
*/
{
    FILE * f;
    int i;
    TestVectorType * testv;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        testv = tv.vcts;
        while ( testv != NULL ) {
            fprintf( f, "%s %s\n", testv->ivct, testv->ovct );
            testv = testv->next;
        }
        fclose(f);
    }
}

void WriteMultiTestFile( char * fn )
/*
    Writes a test file with outputs. Numbers the vectors for one fault (for -D)
*/
{
    FILE * f;
    int i;
    TestVectorType * testv;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        testv = tv.vcts;
        while ( testv != NULL ) {
            fprintf( f, "%i: %s %s\n", testv->no, testv->ivct, testv->ovct );
            testv = testv->next;
        }
        fclose(f);
    }
}

void WriteMultiTestFile_mask( char * fn )
/*
    Writes a test file with outputs and fault mask. Numbers the vectors for one fault (for -D)
*/
{
    FILE * f;
    int i;
    TestVectorType * testv;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        testv = tv.vcts;
        while ( testv != NULL ) {
            fprintf( f, "%i: %s %s ", testv->no, testv->ivct, testv->ovct );
            if ( testv->mask != NULL )
                for ( i = 0; i < nof; i++ )
                    fprintf( f, "%c", testv->mask[i]+'0' );
            fprintf( f, "\n" );
            testv = testv->next;
        }
        fclose(f);
    }
}

void WriteFaultList( char * fn, my_FaultList * fl )
/*
    Writes the reduced faultlist.
*/
{
    FILE * f;
    int i;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        for ( i = 0; i < fl->faults; i++ )
            fprintf( f, "%s\n", fl->names[i] );
        fclose( f );
    }
}

void WriteFaultMask( char * fn, my_FaultList * fl )
/*
    Writes the faultlist mask, according the order in faultlist
    0 = not detected
    1 = detected
    3 = redundant
    4 = aborted
*/
{
    FILE * f;
    int i;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        for ( i = 0; i < fl->faults; i++ )
            fprintf( f, "%c", fl->mask[i]+'0' );
        fclose( f );
    }
}

void WriteUDFaults( char * fn, my_FaultList * fl )
{
    FILE * f;
    int i;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        for ( i = 0; i < fl->faults; i++ )
            if ( fl->mask[i] != 1 )
               fprintf( f, "%s\n", fl->names[i] );
        fclose( f );
    }
}

void WriteABFaults( char * fn, my_FaultList * fl )
{
    FILE * f;
    int i;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        for ( i = 0; i < fl->faults; i++ )
            if ( fl->mask[i] == 4 )
               fprintf( f, "%s\n", fl->names[i] );
        fclose( f );
    }
}

void WriteResults( char * fn, ATPG_Results ar )
{
    FILE * f;
    int i;

    if ( fn != NULL ) {
        f = fopen( fn, "wt" );
        fprintf( f, "gates: %i\n", ar.gates );
        fprintf( f, "iv: %i\n", ar.iv );
        fprintf( f, "ov: %i\n", ar.ov );
        fprintf( f, "i_patterns: %i\n", ar.i_patterns );
        fprintf( f, "patterns: %i\n", ar.patterns );
        fprintf( f, "faults: %i\n", ar.faults );
        fprintf( f, "d_faults: %i\n", ar.d_faults );
        fprintf( f, "r_faults: %i\n", ar.r_faults );
        fprintf( f, "time: %.3f\n", ar.time );

        fclose( f );
    }
}

void Generate_Test()
{
   register int i;
   level LFSR[MAXPI];
   status state;
   FAULTTYPE *f;
   int ndetect3=0;

   tv.num = 0;
   tv.inpvars = nopi;
   tv.outvars = nopo;

   /*****************************************************************
    *                                                               *
    *         step 2: Random pattern testing session                *
    *              1. generate 32 random patterns                   *
    *              2. fault free simulation                         *
    *              3. fault simulation                              *
    *              4. fault dropping                                *
    *                                                               *
    *****************************************************************/

   if(is_random_mode(rptmode)) {

      m_ndetect=random_sim(nog,nopi,nopo,LEVEL,m_nstem,m_stem,LFSR,randomlimit,
	          m_maxbits,maxdetect,&m_ntest,&m_npacket,&m_nbit);

   }

   /******************************************************************
    *                                                                *
    *    step 3: Deterministic Test Pattern Generation Session       *
    *            (fan with unique path sensitization                 *
    *                                                                *
    ******************************************************************/
   reset(phase2);
   fantime=0;
   m_ndetect+=testgen(nog,nopi,nopo,LEVEL,m_maxbits,m_nstem,m_stem,maxbacktrack,
	            phase2,&nredundant,&noverbacktrack,&tbacktrack,
	            &m_ntest,&m_npacket,&m_nbit,&fantime);
   ntest2=m_ntest;

   /******************************************************************
    *                                                                *
    *    step 4: Deterministic Test Pattern Generation Session       *
    *            Phase 2: Employs dynamic unique path sensitization  *
    *                                                                *
    ******************************************************************/

   state=NO_TEST;
   if(maxbacktrack1>0 && nof-m_ndetect-nredundant>0) {
      set(phase2);
      for(i=0;i<nof;i++) {
         f=faultlist[i];
         if(f->detected==PROCESSED) { f->detected=UNDETECTED; }
      }
      fan1time=0;
      m_ndetect+=testgen(nog,nopi,nopo,LEVEL,m_maxbits,m_nstem,m_stem,maxbacktrack1,
                       phase2,&nredundant,&noverbacktrack,&tbacktrack,
	               &m_ntest,&m_npacket,&m_nbit,&fan1time);
      fantime+=fan1time;
   }

   ntest2=m_ntest;

   /********************************************************************
    *                                                                  *
    *       step 5: Test compaction session                            *
    *               32-bit reverse fault simulation                    *
    *               + shuffling compaction   	                       *
    *                                                                  *
    ********************************************************************/

   if(m_ntest==0) {
      ntest3=0;
      ndetect3=0;
   } else if(compact=='n') {
      ntest3=m_ntest;
      ndetect3=m_ndetect;
   } else {
      if(maxcompact==0) compact='r';
      ntest3= compact_test(nog,nopi,nopo,LEVEL,m_nstem,m_stem,nof,&shuf,&ndetect3,m_npacket,m_nbit,m_maxbits);
      if(ndetect3 != m_ndetect) {
         printf("Error in test compaction: m_ndetect=%d, ndetect3=%d\n",m_ndetect,ndetect3);
         exit(0);
      }
   }
}

void help()
{
    printf( "Atalanta-M 1.1b\n\n ");

    printf( "SYNOPSIS: atalanta-M [options] circuit_file\n\n" );
    printf( "OPTIONS:\n\n" );
    printf( "   File specification:\n" );
    printf( "      -n fn      Name of the .bench file. The -n statement is optional, the benchmark name can be specified without any prequisite parameter.\n" );
    printf( "      -f fn      The fault file is read from fn. If not specified, all the s-a-faults are set as default.\n" );
    printf( "      -F fn      The processed fault list is written to a file fn.\n" );
    printf( "      -t fn      Test patterns are are written or read from the file fn (written in TPG mode, read in simulation mode).\n" );
    printf( "      -U fn      Atalanta writes aborted faults to the given file name.\n" );
    printf( "      -v         Atalanta prints out all identified redundant faults as well as aborted fauts in a file. The -U option has to be specified.\n" );
    printf( "      -m fn      The fault mask is written to the file fn. The order of faults corresponds to the fault list.\n" );
    printf( "      -P fn      The ATPG/FS report is written to a file fn.\n" );
    printf( "      -W n       The output test file format (for TPG only).\n" );
    printf( "                   n = 0 - no test pattern output (default).\n" );
    printf( "                   n = 1 - PAT file (test vectors only). Do not use together with -D n option, since all the test vectors are put together.\n" );
    printf( "                   n = 2 - input + output. Do not use together with -D n option, since all the test vectors are put together.\n" );
    printf( "                   n = 3 - more test vectors for each fault, output.\n" );
    printf( "                   n = 4 - more test vectors for each fault, output, with fault mask. HOPE simulator is employed.\n" );
    printf( "\n" );
    printf( "   ATPG Options. These options have no sense in the simulation mode (-S).\n" );
    printf( "      -A         Atalanta derives all test patterns for each fault. In this option, all unspecified inputs are left unknown, and fault simulation is not performed. HOPE fault simulator is employed. Note: it does not work properly. Not all existing test patterns are produced.\n" );
    printf( "      -D n       Atalanta derives n test patterns for each fault. In this option, all unspecified inputs are left unknown, and fault simulation is not performed. If both -A and -D option are specified, -D option is applied. HOPE fault simulator is employed. Note: it does not work properly. Not all existing test patterns are produced.\n" );
    printf( "      -b n        	The number of maximum backtracks for the FAN algorithm phase 1. (default: -b 10)\n" );
    printf( "      -B n       If -B n (n > 0) option is specified, atalanta generates test patterns in two phases. In phase 1, static unique path sensitization is employed. If the test generation for a target fault is aborted in phase 1, the test generation is tried in phase 2. In phase 2, dynamic unique path sensitization is employed. If n=0, phase 2 is not performed. If n > 0, phase 2 test generation is performed with the backtrack limit of n. (default: -B 0, i.e., phase 2 is not performed.)\n" );
    printf( "      -H         HOPE is employed for fault simulation. In this option, three logic values (0, 1 and X), instead of two logic values (0 and 1), are employed. Due to the embedding of the unknown logic value and the parallel fault fault simulation algorithm, the test generation time is slower than the default mode.) (default: FSIM, which is a parallel pattern fault simulator, is employed, and two logic values are used.)\n" );
    printf( "      -L         Static learning is performed. (default: no learning)\n" );
    printf( "      -c n       Atalanta compacts test patterns using two different methods: reverse order compaction and shuffling compaction. First, test patterns are applied in the reverse order and fault simulated (reverse order compaction). Second, test patterns are shuffled randomly and fault simulated (shuffling compaction). During the fault simulations, all the test patterns which do not detect a new fault are eliminated. The option -c n specifies the limit of shuffling compaction. If n>0, shuffling compaction is terminated if n consecutive shuffles do not drop a test pattern. If n=0, shuffling compaction is not included and compaction is done only by the reverse order fault simulation. (default: -c 2)\n" );
    printf( "      -N         Test compaction is not performed.\n" );
    printf( "      -r n       Random Pattern Testing (RPT) Session is included before deterministic test pattern generation session. The RPT session stops if any n consecutive packets of 32 random patterns do not detect any new fault. If n=0, the RPT session is not included. (default: -r 16)\n" );
    printf( "      -s n       Initial seed for the random number generator (random()). If n=0, the initial seed is the current time. (default: -s 0)\n" );
    printf( "      -Z         Atalanta derives one test pattern for each fault. In this option, no fault simulation is performed during the entire test generation (including random pattern test generation session, deterministic test generation session and test compaction session). All unspecified inputs are left unknown.\n" );
    printf( "      -0, -1, -X, -R    During test generation, some inputs can be unspecified. Atalanta provides various options to set these unspecified inputs into a certain value. (default: -R)\n" );
    printf( "   Fault Simulation Options:\n" );
    printf( "      -S          	Simulation mode is performed, instead of TPG. The pattern file has to be specified (-t option), if the -l option is not present.\n" );
    printf( "      -l poly seed num    	Simulates num LFSR patterns. The LFSR polynomial and seed are specified in octal form (poly, seed).\n" );
}

int main(int argc, char **argv)
{
   register int i,j;
   boolean done=FALSE;
   int number;
   int nrestoredfault;
   FAULTTYPE *pcurrentfault,*f;
   GATEPTR gut;
   int k,n,iteration;
   status state,fault_selection_mode;
   int nbacktrack;
   int ndetect3=0;
   int narray[MAXTEST],store=0;
   int lastfault;
   int ncomp=INFINITY,stop=ONE;
   ATPG_Results ar;
   my_FaultList * fl;
   my_FaultList * tfl_1;
   my_FaultList * tfl_2;

   int fault_profile[BITSIZE];
   char c;
   level LFSR[MAXPI],ran;
   int bit=0,packet=0;
   int nd;
   clock_t start, end;
                              
   option_set(argc,argv);

   ReadBenchFile( bench_file );

   if(TFmode == 0)
   {
	SetFaults();
	IndexFaults();
	fl = MakeFaultList();
	if ( w_faults ) WriteFaultList( w_fault_file, fl );

	iseed=Seed(iseed);

	start = clock();

	InitFS();

	if ( simulation_mode ) {			//Fault Simulation
		if ( !LFSR_sim_mode ) {
			ReadTestFile( pattern_file );
			n = SimulateTest();
		} else {
			n = SimulateLFSR();
			ntest2 = LFSR_num;
			ntest3 = LFSR_num;             
		}
		end = clock();
		ar = GetResults();
		ar.time = (end-start)/(double)CLOCKS_PER_SEC;
		UpdateFaultList( fl );
		WriteFaultMask( mask_file, fl );
		WriteResults( report_file, ar);
		if ( ufaultmode == 1 ) WriteABFaults( UD_faults_file, fl );
		else if ( ufaultmode == 2 ) WriteUDFaults( UD_faults_file, fl );
	} else {							//Pattern Generation
		Generate_Test();
		ar = GetResults();
		UpdateFaultList( fl );

		if ( w_test_mode == 4 ) SimulateAllVectors();
		end = clock();
		ar.time = (end-start)/(double)CLOCKS_PER_SEC;
		WriteResults( report_file, ar);
		switch ( w_test_mode ) {
			case 0: break;
			case 1: WriteTestFile( pattern_file );
					break;
			case 2: WriteTestFile_out( pattern_file );
					break;
			case 3: WriteMultiTestFile( pattern_file );
					break;
			case 4: WriteMultiTestFile_mask( pattern_file );
					break;
		}
		WriteFaultMask( mask_file, fl );
		if ( ufaultmode == 1 ) WriteABFaults( UD_faults_file, fl );
		else if ( ufaultmode == 2 ) WriteUDFaults( UD_faults_file, fl );
	}
   }
   /*For transition fault*/
   else	
   {
		
		WriteTFList();

		for(i=0;i<2;i++)
		{
			if(i==0)	
			{
				fault_file = "temp1.flt";
				pattern_file = "temp1.pat";
			}					
			else
			{
				fault_file = "temp2.flt";
				pattern_file = "temp2.pat";
			}
			/*ATPG*/
			SetMode(0);		/*Mode for one-pattern,fault-specified ATPG*/
			
			SetFaults();
			IndexFaults();
			fl = MakeFaultList();
			iseed=Seed(iseed);	
			InitFS();
			Generate_Test();
			UpdateFaultList( fl );
			WriteTestFile( pattern_file );
			
			if(FileEmpty(pattern_file))
			{
				printf("No patterns for this transition fault");
				return 0;
			}
			
			/*Fault Simulation*/		
			SetMode(1);		/*Mode for Fault Simulation*/
			
			SetFaults();	
			IndexFaults();
			if(i==0)	
			{
				tfl_1 = malloc(sizeof(my_FaultList));
				New_MakeFaultList(tfl_1);	
				ReadPattern( pattern_file, i );
			}
			else		
			{
				tfl_2 = malloc(sizeof(my_FaultList));
				New_MakeFaultList(tfl_2);	
				ReadPattern( pattern_file, i );
			}
			
			iseed=Seed(iseed);
			InitFS();
			SetPatFile(pattern_file);		//only one test pattern
			ReadTestFile( pattern_file );
			SimulateTest();
			if(i==0)	UpdateFaultList( tfl_1 );
			else		UpdateFaultList( tfl_2 );

		}
		/*Compare Fault Lists & Write Output File*/
		WriteTFReport(tfl_1,tfl_2);

   }
   /*For transition fault END*/
   //printf("\n Computing time: %.2fs", (end-start)/(double)CLOCKS_PER_SEC);
}


int read_option(char option, char **array, int i, int n)
{
   if(i+1 >= n) return((-1));
   switch(option) {
      case 'd': inputmode='d'; break;
#ifdef ISCAS85_NETLIST_MODE
      case 'I': cctmode=ISCAS85; break;
#endif
      case 'r': sscanf(array[++i],"%d",&randomlimit);           // RPT limit
	        if(randomlimit==0) rptmode='n'; break;
      case 's': sscanf(array[++i],"%d",&iseed); break;          // initial seed
      case 'N': compact='n'; maxcompact=0; break;               // no test compaction
      case 'c': sscanf(array[++i],"%d",&maxcompact); break;     // limit of shuffeling compaction
      case 'b': sscanf(array[++i],"%d",&maxbacktrack); break;   // max. backtracks for FAN
      case 'B': sscanf(array[++i],"%d",&maxbacktrack1); break;  // max. backtracks (2 phases)
      case 't': pattern_file = array[++i]; break;               // test pattern file
   //   case 'l': logmode='y'; strcpy(name3,array[++i]); break;
      case 'n': bench_file = array[++i]; break;                 // .bench file
      case 'L': learnmode='y'; break;                           // static learning
      case 'f': faultmode='f'; fault_file = array[++i]; break;  // source fault file
      case 'F': w_faults = 1; w_fault_file = array[++i]; break;  // destination fault file (complete fault list)
      case 'H': _MODE_SIM='h'; break;                           // HOPE simulation
      case '0': fillmode='0'; break;                            // fill X's with 0s
      case '1': fillmode='1'; break;                            // fill X's with 1s
      case 'X': fillmode='x'; break;                            // don't fill X's
      case 'R': fillmode='r'; break;                            // randomly fill X's
      case 'A': gen_all_pat='y'; break;                         // generate all patterns for each fault
      case 'D': sscanf(array[++i],"%d",&ntest_each_limit);      // debug mode limit
                gen_all_pat='y'; break;
      case 'Z': no_faultsim='y'; break;                         // one test pattern for each fault
  //    case 'u': ufaultmode='y'; break;
      case 'U': ufaultmode = 1; UD_faults_file = array[++i]; break;  // aborted faults file name
      case 'v': ufaultmode = 2; break;                          // undetected faults are printed to a file as well (with -U)
      case 'S': simulation_mode = 1; _MODE_SIM='h'; break;      // perform pattern simulation, not TPG
      case 'm': mask_file = array[++i]; break;                  // mask file name
      case 'P': report_file = array[++i]; break;                // report file name
      case 'W': w_test_mode = array[++i][0] - '0'; break;       // test output file
                                                                //        0 - no test
                                                                //        1 - PAT file
                                                                //        2 - input + output
                                                                //        3 - more test vectors for each fault, output
                                                                //        4 - more test vectors for each fault, output, with fault mask
      case 'l': LFSR_sim_mode = 1;
                sscanf(array[++i],"%s",&LFSR_poly);             // LFSR simulation
                sscanf(array[++i],"%s",&LFSR_seed);
                sscanf(array[++i],"%i",&LFSR_num);
                simulation_mode = 1; _MODE_SIM='h';
                break;
	  /*For transition fault*/
	  case 'T': TFmode = 1;
				TF_line = array[++i];
				TF_type = array[++i];	
				break;
	  /*For transition fault END*/
      default:  i=(-1);
   }
   return(i);
}

int option_set(int argc, char **argv)
{
   int i;


   if(argc==1) {
      help();
   } else
      for(i=1;i<argc;i++) {
         if(argv[i][0]=='-') {
            if((i=read_option(argv[i][1],argv,i,argc))<0)
	       { help();
           break; }
	 }
	 else bench_file = argv[i];
      }

   if(fillmode=='x') _MODE_SIM='h';
   if(gen_all_pat=='y') {
      randomlimit=0;
      rptmode='n';
      maxbacktrack1=0;
      fillmode='x';
      compact='n';
      maxcompact=0;
      _MODE_SIM='h';
      no_faultsim='y';
   }

   if(no_faultsim=='y') {
      randomlimit=0;
      rptmode='n';
      fillmode='x';
      compact='n';
      maxcompact=0;
      _MODE_SIM='h';
   }
   if ( w_test_mode == 4 ) _MODE_SIM = 'h';
}
/*For transition fault*/
void WriteTFList()
{
    FILE * f;
	/*create a temporary fault list*/
	f = fopen( "temp1.flt", "wt" );
	
	if(strcmp(TF_type,"STR") == 0)
		fprintf( f, "%s /1\n", TF_line );
	else
		fprintf( f, "%s /0\n", TF_line );
	
	fclose( f );
     
	f = fopen( "temp2.flt", "wt" );
	
	if(strcmp(TF_type,"STR") == 0)
		fprintf( f, "%s /0\n", TF_line );
	else
		fprintf( f, "%s /1\n", TF_line );
	
	fclose( f );
}

void SetMode(int TFstatus)
{
	if(TFstatus == 0)
	{
		faultmode='f';
		_MODE_SIM='f';
	}
	else
	{
		faultmode='d';			
		_MODE_SIM='h';
	}
}

int FileEmpty(char *name)
{
	int ch;
	FILE * f;
	
	f = fopen( name , "rt" );
	if( (ch=fgetc(f)) == EOF )
	{
		fclose( f );
		return 1;
	}
	else
	{
		fclose( f );
		return 0;
	}	
}

void ReadPattern(char *name, int i)
{
	FILE * f;
	
	f = fopen( name , "rt" );
	if(f == NULL)
		printf("Pattern file doesn't exist\n");
	else if( !feof( f ))
		{
			if(i==0)
				fscanf(f,"%s",pattern1);	//read a vector	
			else
				fscanf(f,"%s",pattern2);
		}
	else	
		printf("Empty Pattern File\n");
	fclose( f );
}

void SetPatFile(char *name)
{
	char s[MAXPI + 1];
	FILE * f;
	
	f = fopen( name , "rt" );
	fscanf(f,"%s",s);
	fclose( f );
	
	f = fopen( name , "wt" );
	fprintf(f,"%s\n",s);
	fclose( f );
}

void GetcctName()
{
	int i;
	
	for(i=0; bench_file[i] != '.'; i++)
		cctname[i] = bench_file[i];
	
	cctname[i] = '\0';
}

void WriteTFReport(my_FaultList * tfl_1, my_FaultList * tfl_2)
{
	int i,j,k,type;
	int otherfaults = 0;
	char s1[50];
	char s2[50];
	FILE * f;	
	GetcctName();
	snprintf(output_file,sizeof(output_file),"%s_%s_%s.txt",cctname,TF_line,TF_type);
	printf("Output file: %s\n",output_file);
	strcpy(s1,TF_line);
	strcat(s1," /1");
	strcpy(s2,TF_line);
	strcat(s2," /0");

	f = fopen( output_file , "wt" );
	
	if(f == NULL)
	{
		printf("Can't open output file\n");
		exit(0);
	}
	
	fprintf(f, "Target Fault: %s %s\n", TF_line, TF_type);	//Target Fault
	fprintf(f, "Initial Pattern: %s\n", pattern1);			//Initial Pattern
	fprintf(f, "Launch Pattern: %s\n", pattern2);			//Launch Pattern
	fprintf(f, "Additional detected faults: ");
	printf("Initial Pattern: %s\n",pattern1);
	printf("Launch Pattern: %s\n",pattern2);
	
	for ( i = 0; i < tfl_1->faults; i++ )
	{
		if( strcmp(s1,tfl_1->names[i])==0 || strcmp(s2,tfl_1->names[i])==0)
			continue;
		for( j = 0; j < tfl_2->faults; j++ )
		{
			if( strcmp(s1,tfl_2->names[j])==0 || strcmp(s2,tfl_2->names[j])==0)
				continue;
			else if( (type = CompareFaults( tfl_1->names[i],tfl_2->names[j] )) != 0 )
			{
				if( (tfl_1->mask[i] == 1) && (tfl_2->mask[j] == 1) )
				{
					otherfaults = 1;
					fprintf(f,"\n  -");
					for(k=0; tfl_1->names[i][k]!=' '; k++)
						fprintf( f, "%c", tfl_1->names[i][k] );
					switch (type)
					{
						case 1:
							fprintf( f, "   \t%s", "STR" );
							break;
						case 2:
							fprintf( f, "   \t%s", "STF" );
							break;
					}	
				}
			}		
		}
	}
	if(otherfaults == 0)
		fprintf(f,"None\n");
	fclose( f );
}

int CompareFaults(char * name1, char * name2)
{
	int i = 0;
	char s1[50], s2[50];
	char ftype1;
	char ftype2;
	
	for(i=0; name1[i]!=' '; i++)
		s1[i] = name1[i];
	s1[i+1] = '\0';	
	ftype1 = name1[i+2];
	
	for(i=0; name2[i]!=' '; i++)
		s2[i] = name2[i];
	s2[i+1] = '\0';	
	ftype2 = name2[i+2];	
	
	if( strcmp(s1,s2)==0 && ftype1!=ftype2 )
	{
		if(ftype1 == '1' && ftype2 == '0')
			return 1;
		else 
			return 2;
	}
	else
		return 0;
}

void New_MakeFaultList(my_FaultList * fl)
{
    int i;
	my_FaultList * fm;
    fm = malloc( sizeof(my_FaultList) );
	fm->faults = nof;
    fm->mask = malloc( nof );
    memset( fm->mask, nof, 0 );
    fm->names = malloc( sizeof(char*) * nof );
    for ( i = 0; i < nof; i++ ) {
        fm->names[i] = malloc( 100 );
        fm->names[i][0] = 0;
        if ( faultlist[i]->line >= 0 ) {
            strcpy( fm->names[i], faultlist[i]->gate->inlis[faultlist[i]->line]->symbol->symbol );
            strcat( fm->names[i], "->" );
        }
        strcat( fm->names[i], faultlist[i]->gate->symbol->symbol );
        strcat( fm->names[i], fault2str[faultlist[i]->type] );
    }
	*fl = *fm;
}
/*For transition fault END*/
