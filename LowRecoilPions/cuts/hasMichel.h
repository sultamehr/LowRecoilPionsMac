//#include "CVUniverse.h"
//#include "Michel.h"
//Cutter written to select Michels 
//PlotUtils includes
#include "PlotUtils/Cut.h"

template <class UNIVERSE, class EVENT>
class hasMichel: public PlotUtils::Cut<UNIVERSE, EVENT>
{
 public:
    hasMichel(): PlotUtils::Cut<UNIVERSE, EVENT>("Event Has Michel ")
    {
    }

  private:
    using Michel = typename std::remove_reference<decltype(std::declval<EVENT>().m_nmichels.front())>::type;

    bool checkCut(const UNIVERSE& univ, EVENT& evt) const
    {
      evt.m_allmichels.clear();
      int nmichels = univ.GetNMichels();
      if (nmichels == 0) continue;
      evt.m_bestdist = 9999.; // setting some default value for best distance
      for (int i = 0; i < nmichels; ++i)
      {
        Michel current_michel = Michel(univ, i);
        if (current_michel.is_fitted != 1) continue;
	//double vtxtime = univ.GetVertex().T()/pow(10, 3); //mus
	//double micheltime = current_michel.time; //already in mus
        //double timediff = micheltime - vtxtime; //if negative, michel happened before vertex. 
        //if (timediff > -.100) continue;
 	if (current_michel.vtx_michel_timediff < 0.400) continue; // < 0.400 is to reject dead michels that happen during dead time. >-0.250 is to see what matches look like for michels that happen before neutrino event. 
        //if (current_michel.true_parentpdg == 211) evt.m_ntruepiparents.push_back(current_michel);  
        //evt.m_nmichels.push_back(current_michel);
        evt.m_allmichels.push_back(current_michel);       
       }
        //Filling Event Level Info needed for 2D selection 
        double lowtpiinevent = univ.GetTrueTpi();
        evt.lowTpi = lowtpiinevent;     
        evt.pT_reco = univ.GetMuonPT();
	evt.q3_reco = univ.Getq3();
	evt.eavail_reco = univ.GetEavail();
        return !evt.m_allmichels.empty();
    }

};
