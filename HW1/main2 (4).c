
//Yasincan Bozkurt 2304202
//Q2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

#define false 0
#define true 1

//Typedefinition of boolean variable
typedef int bool;

struct atom{
    int atomID;
    char atomTYPE; //C, S, T(Th), N or O
    // I showed the element Th with T to make it char 
    };

struct Information {
     int moleculeTYPE; // 1:SO2, 2:CO2, 3: NO2, 4: THO2 ,0: Not Yet
    };

//we declare semaphores for each atom
sem_t semC,semS,semN,semO,semT;
//Semaphore to check if one of thread reads/write atom semaphores
sem_t semAtomAmount;
//Semaphores for every molecule

sem_t CO2,ThO2,NO2,SO2;

sem_t semC_usage,semO_usage,semN_usage,semS_usage, semT_usage;
//Semaphore that will show usthere is generated molecule or not
sem_t semInfo;
//Thread for every molecule
pthread_t CO2_t;
pthread_t NO2_t;
pthread_t SO2_t;
pthread_t TO2_t;
//G parameter 
// it is default 100 given in question
int numberG=100;
//Information object that will hold the information to be printed
struct Information info;    

//This thread will be called when there is a corresponding molecule.
////so2 molecule
void * composer_SO2(void * arg){
	//Since we are going to update semaphores, lock it
	sem_wait(&semAtomAmount);
	//Decrease the semaphore for each atom
	//Increase the semaphore usage
	sem_wait(&semS);
	sem_post(&semS_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	//Increase the molecule semaphore
	sem_post(&SO2);
	sem_post(&semAtomAmount);
	//Set the molecule type so that message will be printed accurate
	info.moleculeTYPE=1;
	//Set the message semaphore so main thread will understand there is a new molecule
	sem_post(&semInfo);
}
//co2 molecule
void * composer_CO2(void * arg){

	sem_wait(&semAtomAmount);
	sem_wait(&semC);
	sem_post(&semC_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_post(&CO2);
	sem_post(&semAtomAmount);
	info.moleculeTYPE=2;
	sem_post(&semInfo);
}
//no2 molecule
void * composer_NO2(void * arg){
	sem_wait(&semAtomAmount);
	sem_wait(&semN);
	sem_post(&semN_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_post(&NO2);
	sem_post(&semAtomAmount);
	info.moleculeTYPE=3;
	sem_post(&semInfo);
}
//tho2 molecule
void * composer_TO2(void * arg){

	sem_wait(&semAtomAmount);
	sem_wait(&semT);
	sem_post(&semT_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_wait(&semO);
	sem_post(&semO_usage);
	sem_post(&ThO2);
	sem_post(&semAtomAmount);
	info.moleculeTYPE=4;
	sem_post(&semInfo);

}

void molecule_finder(int * prioC, int * prioS,int * prioO,int * prioN,int * prioT){
	//Since we will achieve number of atoms by semaphores, we will lock it by semAtomAmount
	int semValS,semValO,semValN,semValC,semValT;//variables that will hold the number of atoms in queue
	sem_wait(&semAtomAmount);
	sem_getvalue(&semS,&semValS);
	sem_getvalue(&semN,&semValN);
	sem_getvalue(&semO,&semValO);
	sem_getvalue(&semC,&semValC);
	sem_getvalue(&semT,&semValT);
	sem_post(&semAtomAmount);
	//Assume that there is no possible molecule in the queue
	bool CO2_b=false;
	bool NO2_b=false;
	bool SO2_b=false;
	bool ThO2_b=false;
	//If there is enough amount, then turn it's variable to true
	if((semValC>=1)&&(semValO>=2)){CO2_b=true;}
	if((semValN>=1)&&(semValO>=2)){NO2_b=true;}
	if((semValS>=1)&&(semValO>=2)){SO2_b=true;}
	if((semValT>=2)&&(semValO>=2)){ThO2_b=true;}
	//count variable will hold the number of possible molecules inside the queue
	//How many possible molecule can be made	
	int count=CO2_b+NO2_b+SO2_b+ThO2_b;
	if(count==1){//if there is only one possible option then create it
		if((semValC>=1)&&(semValO>=2)){pthread_create(&CO2_t,NULL,&composer_CO2,NULL);pthread_join(CO2_t,NULL);}
		else if((semValN>=1)&&(semValO>=2)){pthread_create(&NO2_t,NULL,&composer_NO2,NULL);pthread_join(NO2_t,NULL);}
		else if((semValS>=1)&&(semValO>=2)){pthread_create(&SO2_t,NULL,&composer_SO2,NULL);pthread_join(SO2_t,NULL);}
		else if((semValT>=1)&&(semValO>=2)){pthread_create(&TO2_t,NULL,&composer_TO2,NULL);pthread_join(TO2_t,NULL);}
	} 
	else if(count>1) {//If there is more than one option, then we should look TS values 
		int indexC,indexS,indexO,indexN, indexT;
		sem_wait(&semAtomAmount);//Atom usage will be read, therefore lock the semaphore
		sem_getvalue(&semS_usage,&indexS);
		sem_getvalue(&semN_usage,&indexN);
		sem_getvalue(&semO_usage,&indexO);
		sem_getvalue(&semC_usage,&indexC);
		sem_getvalue(&semT_usage,&indexT);
		sem_post(&semAtomAmount);//Unlock it 
		//There are two possible senario,
		// 1) CO2-NO2-last O is coming
		// 2) ThO2-NO2-last N is coming
int num_selected = CO2_b + NO2_b + ThO2_b + SO2_b;
if (num_selected >= 2) {
    int highest_priority_index;
    if (CO2_b == true) {
        highest_priority_index = indexC;
    }
    if ((NO2_b == true) && (prioN[indexN] > prioC[highest_priority_index])) {
        highest_priority_index = indexN;
    }
    if ((ThO2_b == true) && (prioT[indexT] > prioC[highest_priority_index])) {
        highest_priority_index = indexT;
    }
    if ((SO2_b == true) && (prioS[indexS] > prioC[highest_priority_index])) {
        highest_priority_index = indexS;
    }
    
    if (CO2_b == true) {
        pthread_create(&CO2_t, NULL, &composer_CO2, NULL);
        pthread_join(CO2_t, NULL);
    } else if (NO2_b == true) {
        pthread_create(&NO2_t, NULL, &composer_NO2, NULL);
        pthread_join(NO2_t, NULL);
    } else if (ThO2_b == true) {
        pthread_create(&TO2_t, NULL, &composer_TO2, NULL);
        pthread_join(TO2_t, NULL);
    } else if (SO2_b == true) {
        pthread_create(&SO2_t, NULL, &composer_SO2, NULL);
        pthread_join(SO2_t, NULL);
    }
}



		else{printf("there is an error!\n");}//If there is another senario that will print error message :((
	}
	else{}//no possible molecule
}



//Producing atoms

//Passing arguemment methot
//Method is taken from geeksforgeeks.com
void * Produce_C(void * arg ){
	struct atom * atomPtr = (struct atom *) arg;//	Take argument from main thread
	double waitTime;
	sem_wait(&semAtomAmount);		//Since we are increasing # of C by one, lock the semaphore
	printf("%c with ID: %d is created\n",atomPtr->atomTYPE,atomPtr->atomID);
	sem_post(&semC);			//	Increment semaphore C
        waitTime=((double)rand() / (RAND_MAX)); // create random numberbetween 0 and 1
        waitTime= -log(1-waitTime) / numberG;   // exp dist function tocalculate wait time
	sleep(waitTime);			//	Waiting time 
	sem_post(&semAtomAmount);		//	Unlock it
	}
void * Produce_S(void * arg ){
	double waitTime;
	struct atom * atomPtr = (struct atom *) arg;			//Take argument from main thread
	sem_wait(&semAtomAmount);					//Since we are increasing # of C by one, lock the semaphore
	printf("%c with ID: %d is created\n",atomPtr->atomTYPE,atomPtr->atomID);
	sem_post(&semS);						//	Increment semaphore C	
        waitTime=((double)rand() / (RAND_MAX)); 			// 	create random numberbetween 0 and 1
        waitTime= -log(1-waitTime) / numberG;   			// 	exp dist function tocalculate wait time
	sleep(waitTime);						//	Waiting time 
	sem_post(&semAtomAmount);					//	Unlock it
	}
//Other cases are made within same idea of Carbon or Hydrogen
void * Produce_N(void * arg ){
	struct atom * atomPtr = (struct atom *) arg;
	double waitTime;	
	sem_wait(&semAtomAmount);
	printf("%c with ID: %d is created\n",atomPtr->atomTYPE,atomPtr->atomID);
	sem_post(&semN);
        waitTime=((double)rand() / (RAND_MAX)); 
        waitTime= -log(1-waitTime) / numberG;   
	sleep(waitTime);
	sem_post(&semAtomAmount);
	}
void * Produce_O(void * arg ){
    	double waitTime;
	struct atom * atomPtr = (struct atom *) arg;
	sem_wait(&semAtomAmount);
	printf("%c with ID: %d is created\n",atomPtr->atomTYPE,atomPtr->atomID);
	sem_post(&semO);
        waitTime=((double)rand() / (RAND_MAX)); 
        waitTime= -log(1-waitTime) / numberG;   
	sleep(waitTime);
	sem_post(&semAtomAmount);

	}
	void * Produce_T(void * arg ){
    	double waitTime;
	struct atom * atomPtr = (struct atom *) arg;
	sem_wait(&semAtomAmount);
	printf("%c with ID: %d is created\n",atomPtr->atomTYPE,atomPtr->atomID);
	sem_post(&semT);
        waitTime=((double)rand() / (RAND_MAX)); 
        waitTime= -log(1-waitTime) / numberG;   
	sleep(waitTime);
	sem_post(&semAtomAmount);

	}

int main(int argc, char * argv[]){
	
	int opt;
	int numberM=40;

	while((opt=getopt(argc,argv,":m:g:"))!=-1){
		switch(opt){
			case 'm':
			    numberM=atoi(optarg);
			    break;
			case 'g':
			    numberG=atoi(optarg);
			    break;
			default :
			    break;
		}
	}

	int numberTotalAtoms = numberM;//total nummber of atom
	int numberC=numberM/4;
	int numberS=numberM/4;
	int numberO=numberM/4;
	int numberN=numberM/4;
	int numberT=numberM/4;
	//initialize
	bool conditionGenerate=true;//This will check if generation number is exceed or not
	double waitTime;            //After each generation it will sleep that much time
	int selectionAtom;          // 0: create C ; 1: create H ; 2: create O ; 3: create N
	srand(time(NULL));          // Random generator seed is set to NULL, so that each calling rand function will result in different sequence
	int mAtomID=0;              // atomID


	int priortyListOfC[numberC],priortyListOfS[numberS],priortyListOfN[numberN],priortyListOfO[numberO],priortyListOfT[numberT];

	//Necessary initialization for PTHREADS
	pthread_t arr[numberTotalAtoms];                               
	//Semaphore initialization

	sem_init(&semC,0,0);
	sem_init(&semS,0,0);
	sem_init(&semN,0,0);
	sem_init(&semO,0,0);
	sem_init(&semT,0,0);
	//at start no atom
	// each is zero

	sem_init(&semAtomAmount,0,1);

	sem_init(&CO2,0,0);
	sem_init(&ThO2,0,0);
	sem_init(&NO2,0,0);
	sem_init(&SO2,0,0);
	//no used atom at start
	sem_init(&semC_usage,0,0);
	sem_init(&semO_usage,0,0);
	sem_init(&semN_usage,0,0);
	sem_init(&semS_usage,0,0);
	//zero created molecule  at staart
	sem_init(&semInfo,0,0);
	//Index location 
	int indexC=0;
	int indexO=0;
	int indexS=0;
	int indexN=0;
	int indexT=0;
	//create atom 
	for(int i=0; i<numberTotalAtoms;i++){
		conditionGenerate=true;                 // Reset the flag after each generation
		struct atom nextAtom;                   // atom that will be generated
		mAtomID++;                              // Increment atom ID
		nextAtom.atomID=mAtomID;                //Assign atom nubmer
		while(conditionGenerate){
			selectionAtom=(rand() % 4);         // create random number between 0 and 3
			switch(selectionAtom){
				case 0:                         //If it is C,
					if(numberC==0){break;}      //If C is no longer need in total, do not set the flag
					else{
						conditionGenerate=false;//Set the flag as false since it will be generated
						nextAtom.atomTYPE='C';
						numberC--;		//Decrease the # of C atoms 
						pthread_create(&arr[i],NULL,&Produce_C,&nextAtom);//Create the thread
						priortyListOfC[indexC]=mAtomID;//Set the atomID as TS at the location indexC
						indexC++;		//Increase since a new C atom is generated
					}
					break;
				//Other cases are made within same idea of Carbon
				case 1:
					if(numberS==0){break;}
					else{
						conditionGenerate=false;
						nextAtom.atomTYPE='S';
						numberS--;
						pthread_create(&arr[i],NULL,&Produce_S,&nextAtom);
						priortyListOfS[indexS]=mAtomID;
					indexS++;
					}
					break;
				case 2:
					if(numberO==0){break;}
					else{
						conditionGenerate=false;
						nextAtom.atomTYPE='O';
						numberO--;
						pthread_create(&arr[i],NULL,&Produce_O,&nextAtom);
						priortyListOfO[indexO]=mAtomID;
						indexO++;
					}
					break;
				case 3:
					if(numberN==0){break;}
					else{
						conditionGenerate=false;
						nextAtom.atomTYPE='N';
						numberN--;
						pthread_create(&arr[i],NULL,&Produce_N,&nextAtom);
						priortyListOfN[indexN]=mAtomID;
						indexN++;
					}
				case 4:
					if(numberT==0){break;}
					else{
						conditionGenerate=false;
						nextAtom.atomTYPE='T';
						numberN--;
						pthread_create(&arr[i],NULL,&Produce_T,&nextAtom);
						priortyListOfN[indexT]=mAtomID;
						indexT++;
					}
					break;
			}
		}

		//waits until assig an atom
		pthread_join(arr[i],NULL);
		//Check a molecule creation possible
		molecule_finder(priortyListOfC,priortyListOfS,priortyListOfO,priortyListOfN,priortyListOfT);
		int res;

		sem_getvalue(&semInfo,&res);
		if(res==1){
			switch(info.moleculeTYPE){
				case 1:
					printf("SO2 is generated\n");
					break;
				case 2:
					printf("CO2 is generated\n");
					break;
				case 3:
					printf("NO2 is generated\n");
					break;
				case 4:
					printf("ThO2 is generated\n");
					break;
				default:
					printf("wrong\n");
					break;
			}
			sem_wait(&semInfo);
		}
	
	}
// destructors at the end
	sem_destroy(&semC);
	sem_destroy(&semS);
	sem_destroy(&semN);
	sem_destroy(&semO);
	sem_destroy(&semAtomAmount);
	sem_destroy(&CO2);
	sem_destroy(&ThO2);
	sem_destroy(&NO2);
	sem_destroy(&SO2);
	sem_destroy(&semInfo);
	sem_destroy(&semC_usage);
	sem_destroy(&semO_usage);
	sem_destroy(&semN_usage);
	sem_destroy(&semS_usage);

	return 0;
}
