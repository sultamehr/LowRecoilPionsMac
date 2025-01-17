//File: BestMichelDistance2D.h
//Brief: Require the the distance to vertex for the best Michel electron is below some value.
//Author: Andrew Olivier aolivier@ur.rochester.edu, Mehreen Sultana msultana@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/Cut.h"
#include "event/Michel.h"

template <class UNIVERSE, class EVENT>
class RemoveSignalEvents: public PlotUtils::Cut<UNIVERSE, EVENT>
{
  public:
    RemoveSignalEvents(const double maxDistance): PlotUtils::Cut<UNIVERSE, EVENT>("Per Michel 2D Distance in at least two views is NOT < " + std::to_string(maxDistance) + "mm"), m_maxDistance(maxDistance)
    {
    }

  private:
    double m_maxDistance; //Maximum distance from the vertex that the best Michel can have in mm

    bool checkCut(const UNIVERSE& univ, EVENT& evt) const
    {
      //std::cout << "Implementing Michel Cut with 2D distnace of " << m_maxDistance << " mm" << std::endl;
      std::vector<Michel> nmichelspass;
      int nmichels = evt.m_allmichels.size();
      if (nmichels == 0) return false;
      int npass = 0;
      int nmichelsloop = 0;
      for (unsigned int i = 0; i < evt.m_allmichels.size(); i++)
      {
	nmichelsloop++;

        // For Vertex Match Check to see if 2D distance cut will
        double upvtxXZ = evt.m_allmichels[i].up_to_vertex_XZ;
        double downvtxXZ = evt.m_allmichels[i].down_to_vertex_XZ;
        double upvtxUZ = evt.m_allmichels[i].up_to_vertex_UZ;
        double downvtxUZ = evt.m_allmichels[i].down_to_vertex_UZ;
        double upvtxVZ = evt.m_allmichels[i].up_to_vertex_VZ;
        double downvtxVZ = evt.m_allmichels[i].down_to_vertex_VZ;
       

        std::vector<double> upvtx = {upvtxXZ, upvtxUZ, upvtxVZ};
	std::vector<double> downvtx = {upvtxXZ, upvtxUZ, upvtxVZ};
       
        std::sort(upvtx.begin(), upvtx.end());
	std::sort(downvtx.begin(), downvtx.end());

        //if(upvtx[0] < m_maxDistance && upvtx[1] < m_maxDistance) evt.m_allmichels[i].passable_matchtype.at(0) = 1;
	//if(downvtx[0] < m_maxDistance && downvtx[1] < m_maxDistance) evt.m_allmichels[i].passable_matchtype.at(1) = 2;       
 
        if (upvtxXZ < m_maxDistance && (upvtxUZ < m_maxDistance || upvtxVZ < m_maxDistance)) break;
        else if (upvtxUZ < m_maxDistance && (upvtxXZ < m_maxDistance || upvtxVZ < m_maxDistance)) break;
        else if (upvtxVZ < m_maxDistance && (upvtxXZ < m_maxDistance || upvtxUZ < m_maxDistance)) break;
        else evt.m_allmichels[i].passable_matchtype.at(0) = -1;

        if (downvtxXZ < m_maxDistance && (downvtxUZ < m_maxDistance || downvtxVZ < m_maxDistance)) break;
        else if (downvtxUZ < m_maxDistance && (downvtxXZ < m_maxDistance || downvtxVZ < m_maxDistance)) break;
        else if (downvtxVZ < m_maxDistance && (downvtxXZ < m_maxDistance || downvtxUZ < m_maxDistance)) break;
        else evt.m_allmichels[i].passable_matchtype.at(1) = -1;

        /*
        if (upvtxXZ < m_maxDistance && (upvtxUZ < m_maxDistance || upvtxVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(0) = 1;
        else if (upvtxUZ < m_maxDistance && (upvtxXZ < m_maxDistance || upvtxVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(0) = 1;
        else if (upvtxVZ < m_maxDistance && (upvtxXZ < m_maxDistance || upvtxUZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(0) = 1;
        else evt.m_allmichels[i].passable_matchtype.at(0) = -1;

        if (downvtxXZ < m_maxDistance && (downvtxUZ < m_maxDistance || downvtxVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(1) = 2;
        else if (downvtxUZ < m_maxDistance && (downvtxXZ < m_maxDistance || downvtxVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(1) = 2;
        else if (downvtxVZ < m_maxDistance && (downvtxXZ < m_maxDistance || downvtxUZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(1) = 2;
        else evt.m_allmichels[i].passable_matchtype.at(1) = -1;
        
        double upclusXZ = evt.m_allmichels[i].up_to_clus_XZ;
        double upclusUZ = evt.m_allmichels[i].up_to_clus_UZ;
        double upclusVZ = evt.m_allmichels[i].up_to_clus_VZ;
        double downclusXZ = evt.m_allmichels[i].down_to_clus_XZ;
        double downclusUZ = evt.m_allmichels[i].down_to_clus_UZ;
        double downclusVZ = evt.m_allmichels[i].down_to_clus_VZ;

        std::vector<double> upclus = {upclusXZ, upclusUZ, upclusVZ};
	std::vector<double> downclus = {downclusXZ, downclusUZ,downclusVZ};
   
        std::sort(upclus.begin(),upclus.end());
	std::sort(downclus.begin(),downclus.end());

        //if(upclus[0] < m_maxDistance && upclus[1] < m_maxDistance) evt.m_allmichels[i].passable_matchtype.at(2) = 3;
        //if(downclus[0] < m_maxDistance && downclus[1] < m_maxDistance) evt.m_allmichels[i].passable_matchtype.at(3) = 4;    
      
        
        if (upclusXZ < m_maxDistance && (upclusUZ < m_maxDistance || upclusVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(2) = 3;
        else if (upclusUZ < m_maxDistance && (upclusXZ < m_maxDistance || upclusVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(2) = 3;
        else if (upclusVZ < m_maxDistance && (upclusXZ < m_maxDistance || upclusUZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(2) = 3;
        else evt.m_allmichels[i].passable_matchtype.at(2) = -1;
   
        if (downclusXZ < m_maxDistance && (downclusUZ < m_maxDistance || downclusVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(3) = 4;
        else if (downclusUZ < m_maxDistance && (downclusXZ < m_maxDistance || downclusVZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(3) = 4;
        else if (downclusVZ < m_maxDistance && (downclusXZ < m_maxDistance || downclusUZ < m_maxDistance)) evt.m_allmichels[i].passable_matchtype.at(3) = 4;
        else evt.m_allmichels[i].passable_matchtype.at(3) = -1;
        
        if (evt.m_allmichels[i].passable_matchtype.at(1) != -1 || evt.m_allmichels[i].passable_matchtype.at(2) != -1 || evt.m_allmichels[i].passable_matchtype.at(3) != -1 || evt.m_allmichels[i].passable_matchtype.at(0) != -1)
	{
		npass++;
		continue;

        }
        std::vector<double> distances3D;
  
        if (evt.m_allmichels[i].passable_matchtype[0] == 1) distances3D.push_back(evt.m_allmichels[i].up_to_vertex_dist3D); //Distance between michel to vertex
        if (evt.m_allmichels[i].passable_matchtype[1] == 2) distances3D.push_back(evt.m_allmichels[i].down_to_vertex_dist3D); //distancebetween michel to vertex 
        if (evt.m_allmichels[i].passable_matchtype[2] == 3) distances3D.push_back(evt.m_allmichels[i].up_clus_michel_dist3D); //distnace between michel to cluster
        if (evt.m_allmichels[i].passable_matchtype[3] == 4) distances3D.push_back(evt.m_allmichels[i].down_clus_michel_dist3D); // distance between michel to cluster
        if (distances3D.empty()) distances3D = {9999.,9999.,9999.,9999.};
        //if (distances3D[0] == 9999.) continue; // Comment this line out if you are making efficiency plots
        //std::cout << "Passable Match Types for this Michel are " << evt.m_allmichels[i].passable_matchtype.at(0) << evt.m_allmichels[i].passable_matchtype.at(1) << evt.m_allmichels[i].passable_matchtype.at(2) << evt.m_allmichels[i].passable_matchtype.at(3) << std::endl;
 
        std::sort(distances3D.begin(), distances3D.end());
 
        if (distances3D[0] == 9999.) continue;
        else if (distances3D[0] == evt.m_allmichels[i].up_to_vertex_dist3D){
         //evt.m_allmichels[i].best_XZ = evt.m_allmichels[i].up_to_vertex_XZ;
         //evt.m_allmichels[i].best_UZ = evt.m_allmichels[i].up_to_vertex_UZ;
         //evt.m_allmichels[i].best_VZ = evt.m_allmichels[i].up_to_vertex_VZ;
         //evt.m_allmichels[i].BestMatch = 1;
         //evt.m_allmichels[i].Best3Ddist = evt.m_allmichels[i].up_to_vertex_dist3D;
	 continue;
        //std::cout << "This  Michel is UPVTX and has true endpoint " << evt.m_allmichels[i].trueEndpoint << std::endl;
     	}
        else if (distances3D[0] == evt.m_allmichels[i].down_to_vertex_dist3D){
         //evt.m_allmichels[i].BestMatch = 2;
         //evt.m_allmichels[i].best_XZ = evt.m_allmichels[i].down_to_vertex_XZ;
         //evt.m_allmichels[i].best_UZ = evt.m_allmichels[i].down_to_vertex_UZ;
         //evt.m_allmichels[i].best_VZ = evt.m_allmichels[i].down_to_vertex_VZ;
         //evt.m_allmichels[i].Best3Ddist = evt.m_allmichels[i].down_to_vertex_dist3D;
         //std::cout << "This  Michel is DOWNVTX and has true endpoint " << evt.m_allmichels[i].trueEndpoint << std::endl;
	 continue;
     	}
     	else if (distances3D[0] == evt.m_allmichels[i].up_clus_michel_dist3D){
        //evt.m_allmichels[i].BestMatch = 3;
        //evt.m_allmichels[i].best_XZ = evt.m_allmichels[i].up_to_clus_XZ;
        //evt.m_allmichels[i].best_UZ = evt.m_allmichels[i].up_to_clus_VZ;
        //evt.m_allmichels[i].best_VZ = evt.m_allmichels[i].up_to_clus_UZ;
        //evt.m_allmichels[i].Best3Ddist = evt.m_allmichels[i].up_clus_michvtx_dist3D;
        //std::cout << "This  Michel is UPCLUS and has true endpoint " << evt.m_allmichels[i].trueEndpoint << std::endl;
	 continue;
     	}
     	else if (distances3D[0] == evt.m_allmichels[i].down_clus_michel_dist3D){
        //evt.m_allmichels[i].BestMatch = 4;
        //evt.m_allmichels[i].Best3Ddist = evt.m_allmichels[i].down_clus_michvtx_dist3D;
        //evt.m_allmichels[i].best_XZ = evt.m_allmichels[i].down_to_clus_XZ;
        //evt.m_allmichels[i].best_UZ = evt.m_allmichels[i].down_to_clus_UZ;
        //evt.m_allmichels[i].best_VZ = evt.m_allmichels[i].down_to_clus_VZ;
        //std::cout << "This  Michel is DOWNCLUS and has true endpoint " << evt.m_allmichels[i].trueEndpoint << std::endl;
	 continue;
     	}
     	else{
         	evt.m_allmichels[i].BestMatch = 0;
        	evt.m_allmichels[i].Best3Ddist = 9999.;
        	evt.m_allmichels[i].best_XZ = 9999.;
        	evt.m_allmichels[i].best_UZ = 9999.;
        	evt.m_allmichels[i].best_VZ = 9999.;

		continue;
      	} 
        */
	  //nmichelspass.push_back(evt.m_allmichels[i]);  
        //evt.m_nmichelspass.push_back(evt.m_allmichels[i]);  
     }
      
      //evt.m_nmichels.clear();      
      //evt.m_allmichels.clear(); //empty existing vector of Michels
      //if (!nmichelspass.empty()){
	//evt.selection = 1;
        //std::cout << "Event has a Selection Michel." << std::endl; 
        //evt.m_allmichels.clear();
        //evt.m_nmichels.clear();
        //evt.m_nmichels = nmichelspass; // replace vector of michels with the vector of michels that passed the above cut
      //}
      if (nmichelsloop < nmichels) return false;
      else return true;
     // evt.m_nmichelspass = nmichelspass;
      //return true; 
      //return !evt.m_allmichels.empty();
    }
};
