//Yasincan Bozkurt 2304202
//Q1
// libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

//Definations
#define NUM_ATOMS 5
//assume num_molecules=num_atoms
#define NUM_TUBES 3
#define false 0
#define true 1

typedef int bool;

struct atom{
    int atomID;
    char atomTYPE; //C, S, T(Th), N or O
    // I showed the element Th with T to make it char 
    };
struct tube{
    int tubeID;
    int tubeTS; // time stamp (ID of the atom spilled first)
    int moleculeTYPE; // 1:SO2, 2:CO2, 3: NO2, 4: THO2 ,0: Not Yet
    bool atomsCanAdded[NUM_ATOMS];
    //This array takes true or false to determine whether an atom can be added or not

    int atomNumber;
    int possibleMolecules[NUM_ATOMS];// => {SO2,CO2,NO2,THO2}        same methodology with atomsCanAdded
    char atoms[NUM_ATOMS]; // => Holds atoms inside in a corresponding tube
//  NO ATOM  WILL BE PLACED AS 'E' notation
// this is referring empty
    };
struct Information {
    int tubeID; //where does molecule create
    struct tube mytube ;    //which molecule is generated
    };

bool END_OF_EVERYTHING=false; // This variable holds for the situation ifthe program is finished or not. So that threads can be destroyed

//Global pthread objects
pthread_mutex_t mutex_tubes;    //Mutex corresponds to accessing global tubes variable
pthread_mutex_t mutexInfo;      //Mutex corresponds to accessing global information variable
pthread_cond_t printInfo;       //Condition variable corresponds to starts to print Information
pthread_t tubesUpdate[NUM_TUBES]; //Three threads will update different tube
pthread_cond_t atomNew[NUM_TUBES];  //Condition variable corresponds to update tube (in which atom will be replaced) information
pthread_cond_t updateTube_c;
pthread_t printInfo_t;  //Thread will print molecule information
struct Information info;    //Information object that will hold the information to be printed
struct tube tubes[NUM_TUBES];   // Creating tubes

//Restarting tube in the given index
void restartTube(int index){
    tubes[index].tubeTS=0;
    tubes[index].moleculeTYPE=0;
    tubes[index].atomNumber=0;
    for(int j=0; j<NUM_ATOMS;j++){
            tubes[index].atomsCanAdded[j] =true;
            tubes[index].atoms[j] ='E';
    }

    //possible molecule update does not need 
    // only check atomsCanAdded field when a new atom comes
}
//Initialize tubes 
void initialize(){
    for(int i =0; i<NUM_TUBES;i++){
        tubes[i].tubeID=i+1;
        tubes[i].tubeTS=0;
        tubes[i].moleculeTYPE=0;
        tubes[i].atomNumber=0;
        for(int j=0; j<NUM_ATOMS;j++){
			tubes[i].atomsCanAdded[j] =true;
			tubes[i].atoms[j] ='E';}
        }
    }
//Placing atom into a specified tube (given index)
void placeAtomToTube(int index, char atomType, int atomId){
    for(int i =0;i<NUM_ATOMS;i++){
        if(tubes[index].atoms[i]=='E'){//Finding first empty location in the list
            tubes[index].atoms[i]=atomType;//locate atom into that location
            tubes[index].atomNumber++;
            //increment atom number
            if(tubes[index].atomNumber==1){
                    tubes[index].tubeTS=atomId;
            }
            break;//If an empty location hits, then directly break 
        }
    }

}

//Rreturn # of tube (given) atom can be inserted into.
int possibleLocations(char atomTYPE){
    //This count value counts locations
    int count=0;
    int type=-1;//This will hold the index for the atom type
    switch(atomTYPE){
        case 'C':
            type=0;
            break;
        case 'S':
            type=1;
            break;
        case 'N':
            type=2;
            break;
        case 'O':
            type=3;
            break;
        case 'T':
            type=4;
            break;
        default:
            break;
    }
    for (int j =0; j<NUM_TUBES;j++){//Iterating all tubes
        if(tubes[j].atomsCanAdded[type]==true){//If atom can be inserted
                count++;//Increment count
        }
    }
    return count;
}


// Checks if all tubes are empty or not
bool allTubesEmpty(){
    for(int i =0; i<NUM_TUBES;i++){//Iterating all tubes
            if(tubes[i].atomNumber!=0)
                return false;//If there is a tube with non-zero # of atoms
    }
    //If iteration is finished, then it means all tubes are empty
    return true;
}

// Returns # of (atomType) atom is in the tube (index)
int atom_count_in_tube(int index, char atomType){
    int count=0;
    for (int j =0; j<NUM_ATOMS;j++){//Iterating all the atoms inside a tube
        if(tubes[index].atoms[j]==atomType){//If it hits
                count++;//increment count
        }
    }
    return count;
}

//pprint message to the terminal when a molecule appears
void * printInfo_f(void * arg){
    //Since I will use the global variable,info,
    //Lock the mutex, mutexInfo.
    pthread_mutex_lock(&mutexInfo);

    while(true){//printing loop
        //waİT UNTİL SİGNAL
        pthread_cond_wait(&printInfo,&mutexInfo);
        //If all terminal output occurs, then break the loop so thread can finish
        if(END_OF_EVERYTHING){break;}
        //According to moleculeType, printing message will differ
        switch(info.mytube.moleculeTYPE){
            case 1:
                printf("SO2 is created in tube %d\n",info.tubeID);
                break;
            case 2:
                printf("CO2 is created in tube %d\n",info.tubeID);
                break;
            case 3:
                printf("NO2 is created in tube %d\n",info.tubeID);
                break;
            case 4:
                printf("ThO2 is created in tube %d\n",info.tubeID);
                break;
            default:
                printf("Error! Does not match any case\n");
                break;
        }
    //Finish while loop then unlock the mutex
    pthread_mutex_unlock(&mutexInfo);
    }
}

//This thread corresponds to update the tube fields(*), if there is a new atom in that tube
//Note *: tubeTS, atoms and atomNumber is updated in assignAtom thread function.
//Note **: There are one thread for each tubes.
void * updateTubes(void * arg){
    //Take argument
    int index=*(int*)arg;
    //Since I will use the global variable,tubes,
    //Lock the mutex, mutex_tubes.
    pthread_mutex_lock(&mutex_tubes);
    while(true){

        //If update tube signal(atomNew) is achieved, then go
        //If not, then wait
        pthread_cond_wait(&atomNew[index],&mutex_tubes);
        //If all terminal output occurs, then break the loop so thread can finish
        if(END_OF_EVERYTHING){
                break;
        }
    
        //Each variable corresponds to a molecule
       
        bool possibleSO2=true;
        bool possibleC02=true;
        bool possibleNO2=true;
        bool possibleThO2=true;
        //In this part I avoid confliction by writing several conditions for each elemant and molecule
        for(int i=0;i<tubes[index].atomNumber;i++){//Iterating all atoms inside the tube (until see an empty location)
            switch(tubes[index].atoms[i]){
                case 'C':
                    possibleSO2=false;
                    possibleNO2=false;
                    possibleThO2=false;
                    break;
                case 'S':
                    possibleC02=false;
                    possibleNO2=false;
                    possibleThO2=false;
                    break;
                case 'N':
                    possibleThO2=false;
                    possibleC02=false;
                    possibleSO2=false;
                    break;
                case 'T':
                    possibleC02=false;
                    possibleSO2=false;
                    possibleNO2=false;
                    break;
                default:
                    printf("Error in tube update\n");
                    break;
            }//end of switch block
        }//end of for loop
        //Update the tube field, possible molecules
        tubes[index].possibleMolecules[0]=possibleSO2;
        tubes[index].possibleMolecules[1]=possibleC02;
        tubes[index].possibleMolecules[2]=possibleNO2;
        tubes[index].possibleMolecules[3]=possibleThO2;
        //determine molecule

    
        int amount=0;

        //Initially, assume that none of these atoms can not be inserted
        bool possibleS=false;
        bool possibleC=false;
        bool possibleN=false;
        bool possibleO=false;
        bool possibleT=false;


        //# C is in that tube
        amount=atom_count_in_tube(index,'C');
        if(possibleC02){//If CO2 can be occured
            if(amount < 1){//Since CO2 requires 1 C atom, if amount is lower than 1, then C can be inserted that tube
                possibleC=true;
            }
        }
  
        //Calculate how many S is in that tube
        amount=atom_count_in_tube(index,'S');
        if(possibleSO2){//If SO2 can be occured
            if(amount < 1){//Since SO2 requires 1 S atom, if amount is lower than 1, then C can be inserted that tube
                possibleS=true;
            }
        }
   

        //Calculate how many N is in that tube
        amount=atom_count_in_tube(index,'N');
      
        if(possibleNO2 ){//If NO2 can be occured
            if(amount < 1){possibleN=true;}//Since NO2 requires 1 N atom, if amount is lower than 1, then N can be inserted that tube
        }
   
        //Calculate how many N is in that tube
        amount=atom_count_in_tube(index,'T');
      
        if(possibleThO2 ){//If ThO2 can be occured
            if(amount < 1){possibleT=true;}//Since ThO2 requires 1 Th atom, if amount is lower than 1, then N can be inserted that tube
        }
   
        //Calculate how many Th is in that tube
        amount=atom_count_in_tube(index,'T');
        if(possibleThO2){//If CO2 can be occured
            if(amount < 1){//Since ThO2 requires 1 Th atom, if amount is lower than 1, then C can be inserted that tube
                possibleT=true;
            }
        }
      
        //Calculate how many O is in that tube
        amount=atom_count_in_tube(index,'O');
        if(possibleNO2 ){//If NO2 can be occured
            if(amount < 2){possibleO=true;}//Since NO2 requires 2 O atom, if amount is lower than 2, then O can be inserted that tube
        }
       
        if(possibleC02 ){//If CO2 can be occured
            if(amount < 2){possibleO=true;}//Since CO2 requires 2 O atom, if amount is lower than 2, then O can be inserted that tube
        }
           if(possibleSO2 ){//If CO2 can be occured
            if(amount < 2){possibleO=true;}//Since SO2 requires 2 O atom, if amount is lower than 2, then O can be inserted that tube
        }
           if(possibleThO2 ){//If Th02 can be occured
            if(amount < 2){possibleO=true;}//Since Th02 requires 2 O atom, if amount is lower than 2, then O can be inserted that tube
        }

        //Update the tube field, atomsCanAdded
        tubes[index].atomsCanAdded[0]=possibleC;
        tubes[index].atomsCanAdded[1]=possibleS;
        tubes[index].atomsCanAdded[2]=possibleN;
        tubes[index].atomsCanAdded[3]=possibleO;
        tubes[index].atomsCanAdded[4]=possibleT;
        //check whetherif a molecule can be generated
  
        //If none of atoms can be added to this tube, then it means we must create molecule (there is no possible space on that module)
        bool anAtomCanAdd=possibleC || possibleS || possibleN || possibleO || possibleT;
        if(!(anAtomCanAdd) ){//If anAtomCanAdd is false, that means we can not add an atom to that tube == Molecule must be generated
            int molecule=-1;
            for (int j =0; j<NUM_ATOMS;j++){//Iterate all molecule type
                if(tubes[index].possibleMolecules[j]==true){//If that molecule can be created
                        molecule=j;//Hold that value
                        break;
                }
            }
     
            pthread_mutex_lock(&mutexInfo);
            //change info
            info.tubeID=index+1;
            info.mytube.moleculeTYPE=molecule+1;
            //Unlock the mutexInfo since it is updated
            pthread_mutex_unlock(&mutexInfo);
            //Since molecule is created,we should clear the tube
            restartTube(index);
            //start the printer thread
            pthread_cond_signal(&printInfo);
	    
        }
       
    }
    //unlock the mutex since we finished all tasks
    pthread_mutex_unlock(&mutex_tubes);
    //malloc is used so free that space
    free(arg);
}



//Decidee  cube to generated atom into

void * assignAtom(void * arg ){
    //place variable holds the index of tube
    int place=-1;

    //Taking argument,atom, from main thread
    struct atom * atomPtr = (struct atom *) arg;
    //Since we are accessing global variables, cubes, we should lock this mutex
    pthread_mutex_lock(&mutex_tubes);
    //calculating number of possible location in all tubes
    int numberOfLocation=possibleLocations(atomPtr->atomTYPE);
    //In case no possible location then it is wasted.
    if(numberOfLocation==0){
        pthread_mutex_unlock(&mutex_tubes);
        pthread_exit(NULL);//send NULL to main thread so that it can understand it is wasted
    }
    //If not
    else{
        double smallestTS=9999999999;//Assigning an arbitrary huge number
        //Find index of atomsCanAdded 
        int type=-1;
        switch(atomPtr->atomTYPE){
            case 'C':
                type=0;
                break;
            case 'S':
                type=1;
                break;
            case 'N':
                type=2;
                break;
            case 'O':
                type=3;
                break;
            case 'T':
                type=4;
                break;
            default:
                break;
        }

        for(int i=0;i<NUM_TUBES;i++){
            if(tubes[i].atomsCanAdded[type]==true){
                if((tubes[i].tubeTS < smallestTS)&&(tubes[i].tubeTS!=0)){//Find the non-empty and highest-prior tube
                    smallestTS=tubes[i].tubeTS;
                    place=i;

                }
            }
        }
        
        if(place==-1){
            for(int i=0;i<NUM_TUBES;i++){//Iterate all tubes
                if(tubes[i].atomsCanAdded[type]==true){//Through iteration, if you see an available tube,
                    place=i;//Take that value
                    break;//kill the iteration 
                }
            }
        }//end if
	    //Update thee tube fields of tubeTS, atoms and atomNumber
	    
    }//end else 
    placeAtomToTube(place,atomPtr->atomTYPE,atomPtr->atomID);

    //There will be no longer tube accessing
    pthread_mutex_unlock(&mutex_tubes);

    //To give the result, assign a dynamic memmory
    int *result=malloc(sizeof(int));
    *result=place+1;
    pthread_exit((void*)result);


}
// In this part I used geeksforgeeks.com to get an insight.
int main(int argc, char * argv[]){
    //  Taking input from terminal
    int opt;
    int numberC=20,numberS=20,numberO=20,numberN=20,numberT=20,numberG=100;

    while((opt=getopt(argc,argv,":c:s:o:n:t:g:"))!=-1)
    {
        switch(opt)
        {
        case 'c':
            numberC=atoi(optarg);
            break;
        case 's':
            numberS=atoi(optarg);
            break;
        case 'o':
            numberO=atoi(optarg);
            break;
        case 'n':
            numberN=atoi(optarg);
            break;
            case 't':
            numberT=atoi(optarg);
            break;
        case 'g':
            numberG=atoi(optarg);
            break;
        default :
            break;
        }
    }

    int numberTotalAtoms = numberC+numberS+numberO+numberN+numberT;//Total # of atoms

    //Necessary initialization
    bool conditionGenerate=true;//This will check if generation whehther number is exceed or not
    double waitTime;            //After each generation it will sleep that much time
    int selection_atom;          
    srand(time(NULL));          // Random generator seed is set to NULL, so that each calling rand function will result in different sequence
    int mAtomID=0;              // atomID
    int *res;                   // Taking Result

    //info initializer
    info.tubeID=0;
    info.mytube.tubeID=0;
    info.mytube.tubeTS=0;
    info.mytube.moleculeTYPE=0;
    info.mytube.atomNumber=0;

    //Necessary initialization for PTHREADS
    pthread_t arr[numberTotalAtoms];                               //ATOM ASSIGN THREAD

    pthread_mutex_init(&mutex_tubes,NULL);          //init mutex

    pthread_cond_init(&atomNew[0],NULL);            
    pthread_cond_init(&atomNew[1],NULL);            
    pthread_cond_init(&atomNew[2],NULL);            
    pthread_cond_init(&printInfo,NULL);             
    //pthread_cond_init(&updateTube_c,NULL);             //init cond

    pthread_create(&printInfo_t,NULL,&printInfo_f,NULL);//Printer thread is created(it will wait)

    //To create a thread for each tube
    for(int i=0;i<NUM_TUBES;i++){
        // To assign the tube ID, we should pass argument
        int *a=malloc(sizeof(int));//create dynamically
        *a=i;
        pthread_create(&tubesUpdate[i],NULL,&updateTubes,a);    //Free it with free(arg) inside in the tube thread
    }

    //Initialize tubes
    initialize();


    //creatE ATOM 
    
    for(int i=0; i<numberTotalAtoms;i++)//Iterate as
    {
        waitTime=((double)rand() / (RAND_MAX)); // create random number between 0 and 1
        waitTime= -log(1-waitTime) / numberG;   // exp dist function to calculate wait time
        conditionGenerate=true;                 // Reset the flag after each generation

        struct atom nextAtom;                   // atom that will be generated
        mAtomID++;                              // Increment atom ID
        while(conditionGenerate){
            selection_atom=(rand() % 5);         // create random number between 0 and 4
            switch(selection_atom){
                case 0:                         //If it is C,
                    if(numberC==0){break;}      //If C is no longer need in total, do not set the flag
                    else{
                        nextAtom.atomTYPE='C';  //generate atom
                        numberC--;              //decrement the number of C in total
                        }
                    conditionGenerate=false;     //set the flag so that break that loop then assign it to a tube
                    break;
                //Other cases are made within same idea of Carbon

                case 1:
                    if(numberS==0){break;}
                    else{
                        nextAtom.atomTYPE='S';
                        numberS--;
                        }
                    conditionGenerate=false;
                    break;
                case 2:
                    if(numberO==0){break;}
                    else{
                        nextAtom.atomTYPE='O';
                        numberO--;
                        }
                    conditionGenerate=false;
                    break;
                case 3:
                    if(numberN==0){break;}
                    else{
                        nextAtom.atomTYPE='N';
                        numberN--;

                        }
                case 4:
                    if(numberT==0){break;}
                    else{
                        nextAtom.atomTYPE='T';
                        numberT--;

                        }
                    conditionGenerate=false;
                    break;
            }
            nextAtom.atomID=mAtomID;//assign atom nubmer


        }
	
        pthread_create(&arr[i],NULL,&assignAtom,&nextAtom);
        
        //wait until finish
        pthread_join(arr[i],(void**)&res);//Wait finish
	sleep(waitTime);
        printf("%c with ID:%d is created.\n",nextAtom.atomTYPE,nextAtom.atomID);//atom generated 
        if(res == NULL){
		printf("%c with ID:%d wasted.\n",nextAtom.atomTYPE,nextAtom.atomID);
	}
	// message print
        else{
		pthread_cond_signal(&atomNew[(*res)-1]);		
	}
    }
    //If whole process is ended, change the terminater variable to true
    END_OF_EVERYTHING=true;
 
    pthread_cond_signal(&atomNew[0]);
    pthread_cond_signal(&atomNew[1]);
    pthread_cond_signal(&atomNew[2]);
    pthread_cond_signal(&printInfo);
    //Be sure they are ended
    pthread_join(printInfo_t,NULL);
    for(int i=0;i<NUM_TUBES;i++){
        pthread_join(tubesUpdate[i],NULL);
    }
    //DESTROYING PTHREAD OBJECTS
    pthread_mutex_destroy(&mutex_tubes);
    pthread_cond_destroy(&atomNew[0]);
    pthread_cond_destroy(&atomNew[1]);
    pthread_cond_destroy(&atomNew[2]);
    //Free the allocated dynamic memory
    free(res);
    return 0;

}
