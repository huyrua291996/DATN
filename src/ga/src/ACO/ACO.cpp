/* iPath: A C++ Library of Intelligent Global Path Planners for Mobile Robots with ROS Integration. 

 * Website: http://www.iroboapp.org/index.php?title=IPath
 * Contact: 
 *
 * Copyright (c) 2014
 * Owners: Al-Imam University/King AbdulAziz Center for Science and Technology (KACST)/Prince Sultan University
 * All rights reserved.
 *
 * License Type: GNU GPL
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include"ACO.h"
#include <vector>
#include <limits>

using namespace std;
ofstream file100 ("liste.txt", ios::trunc) ;

/**********************************************************/
//Constructors and destructors
//Check Status: Checked by Imen 
/**********************************************************/
ACO::ACO(void)
{
}
ACO::ACO(list <pheromone> pheromoneL)
{
   setPheromoneList(pheromoneL);
}
ACO::ACO(long double initPheromoneValue ,long double alpha , long double beta , long double evaporationRate ,int Q , unsigned int numberOfAnts,int numberOfIterations):GenericACO(initPheromoneValue,alpha,beta,evaporationRate,Q,numberOfAnts, numberOfIterations)
{  
}
ACO::~ACO(void)
{
}
/**********************************************************/
//Function: Mutators
//Check Status: Checked by Imen 
/**********************************************************/
void ACO::setPheromoneList (list <pheromone> pheromoneL)
{
	PheromoneList=pheromoneL;
}
/**********************************************************/
//Function: Accessors
//Check Status: Checked by Imen 
/**********************************************************/
list <pheromone>  ACO::getPheromoneList()
{
	return PheromoneList;
}
/**********************************************************/
// Function initializePheromoneList
// it used to initialze the list of pheromone 
// Input: the occupancy grid map
//Check Status: Checked by Imen 
/**********************************************************/
void ACO::initializePheromoneList(OccupancyGridMap* map )
{ 
  pheromone* initialPheromone= new pheromone();
  vector<int> freeNeighborCells;
  
  for (int i=0; i< map->getWidth(); i++)
   {
     for(int j=0; j<map->getHeight(); j++)
     {
       if (map->isFree(i,j)) // for all the free cells
       { 
	 int index=map->getCellIndex(i,j);
	 vector<int> freeNeighborCells=findFreeNeighborCell(map,index);
	 uint q=0;
         while (q<freeNeighborCells.size())
         { 
	   if(map->getCellRowID(index) > map->getCellRowID(freeNeighborCells[q]) || (map->getCellRowID(index) == map->getCellRowID(freeNeighborCells[q]) && map->getCellColID(index) > map->getCellColID(freeNeighborCells[q])) )
	   {
	      initialPheromone->setFromCell(index);
	      initialPheromone->setToCell(freeNeighborCells[q]);
	      initialPheromone->setPheromone(getInitialPheromoneValue()) ;
	      PheromoneList.push_back(*initialPheromone);
	   }
	   q++;
	 }
      
       }
     }
   }   
}
/**********************************************************/
// Function computeTransitionProbabilities
// it calculates the transition probabilities of the ants between the cells
// Input: the map, the current Cell and the Unvisited Neighbor Cells of the current cell.
// Output: the probabilities calculated by the transition rule probability.
//Check Status: Checked by Imen 
/**********************************************************/
vector <long double> ACO::computeTransitionProbabilities(OccupancyGridMap* map,int currentCell,vector <int> unvisitedNeighborCells,int iteration)
{
        vector <long double> probabilities; //it contains the probabilities of transition
	long double etha; // the inverse of euclidian distance between the current cell and the neighbor cell
	
	//calculate the probabilities using the transition rule probability
	
	for (uint i=0; i<unvisitedNeighborCells.size() ; i++)
	{
	      etha = 1/map->getMoveCost(currentCell,unvisitedNeighborCells[i]);
	      probabilities.push_back(powl(getPheromoneValue(currentCell,unvisitedNeighborCells[i]),getAlpha()) * powl(etha,getBeta()));
        }
        long double sum=0;
        for (uint i=0; i< probabilities.size(); i++)
	{
	  sum=sum+probabilities[i];
	}
	
	if (sum != 0)     
	  for (uint i=0; i<unvisitedNeighborCells.size() ; i++)
	    {
		probabilities[i] = probabilities[i]/sum ;
	    }
	
	return probabilities;
}
/**********************************************************/
// Function getPheromoneValue
// it used to obtain a pheromone value of an arc from the pheromone list 
// Input: two cells FromCell and ToCell 
// Output: the pheromone value on the edge (FromCell,ToCell)
//Check Status: Checked by Imen 
/**********************************************************/
long double ACO::getPheromoneValue(int FromCell,int ToCell)
{
  long double pheromoneValue;
  
    for(list<pheromone>::iterator it = PheromoneList.begin(); it !=PheromoneList.end(); it++)
    {
      if((((*it).getFromCell()==FromCell) && ((*it).getToCell()==ToCell)) || (((*it).getFromCell()==ToCell) && ((*it).getToCell()==FromCell)))
      {
	pheromoneValue=(*it).getPheromone();
      }
    }
  
  return pheromoneValue;
}
/**********************************************************/
// Function updatePheromone
// it is used to update the quantity of pheromone.
// input: vector of Ant that include the ants id, their paths and their paths cost.
// output:
//Check Status: Checked by Imen 
/**********************************************************/
void ACO::updatePheromone(vector <Ant*> ants)
{  
   long double delta; // the quantity of pheromone laid on edge (cell1, cell2) joining two cells i and j by all ants
   if(ants.size()!=0)
   {
      for(list<pheromone>::iterator it = PheromoneList.begin(); it !=PheromoneList.end(); it++)
	{   
	    delta=getDelta(ants,(*it).getFromCell(),(*it).getToCell());
	    (*it).setPheromone(getPheromoneValue((*it).getFromCell(),(*it).getToCell()) * (1-getEvaporationRate()) + delta);
	}
      }	
}
/**********************************************************/
// Function getDelta
// It is used to calculate the delta value used in the update pheromone rule.
// Input: list of ants the index of the cells i and j.
// Output: the delta value.
//Check Status: Checked by Imen 
/**********************************************************/
long double ACO::getDelta(vector <Ant*> Ants, int cell1, int cell2)
{
  long double delta = 0; 
  long double Lk =0; // cost of the constructed path by all the ants .
  bool exist = false;
  for (uint k = 0; k < Ants.size(); k++)  
   {
      if(Ants[k]->getConstructedPath()->getCost()!=0)
      {
	    for (uint l = 0; l< Ants[k]->getConstructedPath()->getPath().size()-1; l++)
	    {
	    // if the edge relying two cells cell1 and cell2
	      if ((( Ants[k]->getConstructedPath()->getPath()[l] == cell1) && ( Ants[k]->getConstructedPath()->getPath()[l+1] == cell2)) || (( Ants[k]->getConstructedPath()->getPath()[l] == cell2) && ( Ants[k]->getConstructedPath()->getPath()[l+1] == cell1))) 
		{
		    // if the ant k uses the edge joining the two cells cell1 and cell2
		    // the length of the path constructed by the ant k is added to Lk value.
		    Lk = Lk + Ants[k]->getConstructedPath()->getCost();
		    exist=true;
		    break;
		}
	    }
      }
     }
	

	if (exist == true) 
	{   
		// if the edge (cell1,cell2) is used by the ants then the value of delta is different from 0
		//thus the quantity of pheromone will increase else it will evaporate
		delta = getQ()/Lk;
	}
 
	return delta;
}
/**********************************************************/
// Function getNextCell
// it is used to select the next cell of the ant.
// input: a vector of neighbor cells and their probabilities.
// output: the next cell of the ant.
//Check Status: Checked by Imen 
/**********************************************************/
int ACO::getNextCell(vector <int> NeighborCells, vector < long double> probabilities)
{
  int nextCell;
  vector < long double> cumulativeProbabilities;
  float randomNumber;
  float inverseRandomNumber;
  
  //calculate the cumulative probabilities
  cumulativeProbabilities.push_back(probabilities[0]);
  for(uint i=1; i<probabilities.size();i++)
  {
    cumulativeProbabilities.push_back(cumulativeProbabilities[i-1]+probabilities[i]);
  }
               file100 << "Cumulative Probabilities: ";
		for(uint i=0; i<cumulativeProbabilities.size();i++)
		    file100 << cumulativeProbabilities[i] << "\t";
		file100<<endl;
  //choose a random number
  randomNumber =(float)rand() / (float)RAND_MAX;
  //inverse of the random number
  inverseRandomNumber=1-randomNumber;
 
     file100 << "Random number= " << inverseRandomNumber << endl;
  // choose the next cell
    //case1:
    if (inverseRandomNumber >= 0 && inverseRandomNumber <= cumulativeProbabilities[0])
    {
      nextCell=NeighborCells[0];
    }
    else
    {
        //case2:
        if(inverseRandomNumber >= cumulativeProbabilities[cumulativeProbabilities.size()-1] && inverseRandomNumber <= 1)
        {
           nextCell=NeighborCells[NeighborCells.size()-1];
	}
	//case3:
        else
	{
	     for(int i=0; i< cumulativeProbabilities.size(); i++)
	     {
	        if(inverseRandomNumber >= cumulativeProbabilities[i] &&  inverseRandomNumber <= cumulativeProbabilities[i+1])
		{
                   long double difference1=inverseRandomNumber- cumulativeProbabilities[i];
                   long double difference2=cumulativeProbabilities[i+1]- inverseRandomNumber;
                   if (difference1< difference2) 
		      nextCell=NeighborCells[i];
                   else
		      nextCell=NeighborCells[i+1];
		}

	     }
	}
     }
   return  nextCell ;
}
/*******************************************************************************/
//Function Name: findPath
//Inputs: the map, the start cell and the goal cell
//Output: the best path of the algorithm
//Description: it is used to generate the best path after N iterations 
//Check Status: Checked by Imen 
/*********************************************************************************/
Path* ACO::findPath(OccupancyGridMap* map, int startCell, int goalCell, bool enable_trace)
{
  if (enable_trace==true)
 {
   file100 << "Planner Type -ACO-" << endl;
   file100 << "/....Map dimensions" << endl;
   file100 << "Width= " << map->getWidth() << " " << "Height= " << map->getHeight() <<endl;
   file100 << "/....Start and goal positions" << endl;
   file100 << "The start position is: " <<startCell << " and the goal position is: " << goalCell << endl;
   file100 << "/....ACO parameters" << endl;
   file100 << "Alpha= " << getAlpha() << " " << "beta= " << getBeta() << " " << "Evaporation rate= " << getEvaporationRate() << " " << "Q= " << getQ() << " " << "Number of ants= " << getNumberOfAnts() << " " << "Initial pheromone value= " << getInitialPheromoneValue() << endl;
   file100 << "/....starting simulation" << endl;
 }
 Path* bestPath=new Path(); //the best path
 vector<Path*> bestPaths; // it contains the best paths generated after each iteration
 vector <int> neighborCell; //the vector of neighbor cells
 vector <int> unvisitedFreeNeighborCell; //the unvisited neighbor cells
 int currentCell; //the current  position of the ant
 int nextCell; //the new position  of the ant
 vector <long double> probabilities ; // it contains the probabilities of the possible next positions of an ant
 vector<Ant*> Ants(getNumberOfAnts()); // vector of ant
 int indexBestPath; // index of the best path of the algorithm
 
 
 //create the ants
 for(uint j=0 ; j<getNumberOfAnts();j++)
      Ants[j]=new Ant();
      
 //intialize the pheromone list
 initializePheromoneList(map);
 if (enable_trace==true)
   {
      file100 << "The initial pheromone List: "<< endl;
      for(list<pheromone>::iterator it = PheromoneList.begin(); it !=PheromoneList.end(); it++)
	{ 
	  file100 << "cell1= " << (*it).getFromCell() << "cell2= " <<  (*it).getToCell() << "Pheromone= " << (*it).getPheromone() << endl;
	}
	file100 << endl;
   }

 for ( int i=0; i< getNumberOfIterations() ; i++) //ACO is repeated "numberOfIterations" times
   {
     if(enable_trace==true)
     {
        file100 << "**********************************" << endl;
        file100 << "iteration " << i << endl;
	 file100 << "**********************************" << endl;
     }
     for(uint j=0 ; j<getNumberOfAnts();j++) // for each Ant
     {
      currentCell=startCell;
      Path* pathAnt=new Path();
      Ants[j]->setPosition(currentCell);
      pathAnt->insertCell(map,0,currentCell);
      Ants[j]->setConstructedPath(pathAnt);
     int index=1;
      //while the ant has not reached the goal position or not blocked
      while  (currentCell != goalCell)
      {  
         neighborCell=findFreeNeighborCell(map,currentCell);
         unvisitedFreeNeighborCell=getUnvisitedFreeNeighbors(neighborCell,pathAnt);
	 if(enable_trace==true)
	 {
	   file100 << "The current Cell " << currentCell << endl;
	   file100 << "The unvisited neighbor cells: ";
	   for(uint i=0; i<unvisitedFreeNeighborCell.size();i++)
	     file100 << unvisitedFreeNeighborCell[i] << "\t";
	   file100<<endl;
	 }
         if (unvisitedFreeNeighborCell.size() == 0)// if the ant is blocked and there is no unvisited neighbor cells so the ant will be discarted 
           {
              pathAnt->getPath().clear();
	      Ants[j]->setConstructedPath(pathAnt);
	      Ants[j]->getConstructedPath()->setCost(0);
	     if(enable_trace==true)
	      {
		file100<<"The ant [ "<<j<< "] is blocked" << endl;
	         file100 << "**********************************************************************************" << endl;
	      }
	      break;
	    }
	 else
	   {     
	      probabilities=computeTransitionProbabilities(map,currentCell,unvisitedFreeNeighborCell,i);
	      if(enable_trace==true)
	      {
		file100 << "Probabilities: ";
		for(uint i=0; i<probabilities.size();i++)
		    file100 << probabilities[i] << "\t";
		file100<<endl;
	      }
	      nextCell=getNextCell(unvisitedFreeNeighborCell,probabilities);
	      if(enable_trace==true)
	      {
		file100 << "The next cell is: "<< nextCell << endl;
	        file100 << "------------------------" << endl;
	      }
	      currentCell = nextCell;
	      pathAnt->insertCell(map,index,nextCell);	      
	      index++;
	   
	      if (currentCell == goalCell) // if the ant has reached the goal cell
	        { 
		  if(enable_trace==true)
		    file100 << "The ant reachs the goal position" << endl;
		  Ants[j]->setConstructedPath(pathAnt);
		  Ants[j]->getConstructedPath()->setCost(pathAnt->getPathCost(map,false)); 
		  if(enable_trace==true)
		  {
		    file100 << "The path of the ant [" << j << "] " ;
		    for(uint i=0; i<Ants[j]->getConstructedPath()->getPath().size();i++)
		      file100 << Ants[j]->getConstructedPath()->getPath()[i] << "\t";
		    file100 << endl;
 		    //  Ants[j]->getConstructedPath()->printPath();
		    file100 << "The path cost is " << Ants[j]->getConstructedPath()->getCost() << endl;
		     file100 << "*******************************************************************************" << endl;
 		    
		  }
		
		 } 
	    }
       }
       pathAnt->getPath().clear(); 
     }
 
 // The best path after one iteration
     Path* bestPathInAnIteration=new Path();
     bestPathInAnIteration= findBestAnt(map,Ants);
      
     if(bestPathInAnIteration->getCost()!=0)
     {
	cout<<"The best path after iteration " << i << endl;
	bestPathInAnIteration->printPath();
	cout << "Its cost is " <<bestPathInAnIteration->getPathCost(map,false) << endl;
	bestPaths.push_back(bestPathInAnIteration);
     }
 
   // update the pheromone list
   updatePheromone(Ants);
   if (enable_trace==true)
   {
      file100 << "The updated pheromone Matrix is: "<< endl;
      for(list<pheromone>::iterator it = PheromoneList.begin(); it !=PheromoneList.end(); it++)
	{ 
	  file100 << "cell1= " << (*it).getFromCell() << "cell2= " <<  (*it).getToCell() << "Pheromone= " << (*it).getPheromone() << endl;
	}
	file100 << endl;

   } 
 }
  
  // The best path generated by the algorithm
  if(bestPaths.size()!=0)
  {  
     indexBestPath=findBestPath(map,bestPaths); 
     bestPath->setPath(bestPaths[indexBestPath]->getPath());
     bestPath->setCost(bestPaths[indexBestPath]->getPathCost(map,false));
  }
 
 return bestPath ;
}
















