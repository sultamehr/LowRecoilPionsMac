//This is what we'll do for reco
/*template <class UNIVERSE>
using q3Signal = PlotUtils::Maximum<UNIVERSE, double, &UNIVERSE::GetTrueq3>;*/ //TODO: Except I didn't actually write this for SignalConstraints :(

//PlotUtils includes
#include "PlotUtils/Cut.h"

//Package includes
#include "event/CVUniverse.h"

template <class UNIVERSE>
class Q3Limit: public PlotUtils::SignalConstraint<UNIVERSE>
{
  public:
    Q3Limit(const double q3Min, const double q3Max): PlotUtils::SignalConstraint<UNIVERSE>(std::to_string(q3Min) + "True q3 < " + std::to_string(q3Max) + " GeV"),
  fQ3Min(q3Min),
  fQ3Max(q3Max)
    {
    }

  private:
    double fQ3Max; //Maximum q3 allowed in GeV/c
    double fQ3Min; // Min q3 allowed in GeV/c
    bool checkConstraint(const UNIVERSE& univ) const //override
    {
      double trueq3 = univ.Getq3True();
      return trueq3 > fQ3Min && trueq3 <= fQ3Max;
    }
};
template <class UNIVERSE>
class hasTruePion: public PlotUtils::SignalConstraint<UNIVERSE>
{
  public:
    hasTruePion(): PlotUtils::SignalConstraint<UNIVERSE>("has at least 1 True Pion")
    {
    }

  private:
    bool checkConstraint(const UNIVERSE& univ) const //override
    {

      int npi = 0;
      std::vector<int> npart = univ.GetTrueFSPDGCodes();
      for (auto pdg : npart)
      {
         if (pdg == 211) npi++;

      }

      if (npi >= 1 && npi <= 3) return true;
      else return false;
    }
};

template <class UNIVERSE>
class pTRangeLimit: public PlotUtils::SignalConstraint<UNIVERSE>
{
  public:
    pTRangeLimit(const double pTMin, const double pTMax): PlotUtils::SignalConstraint<UNIVERSE>(std::to_string(pTMin) + "True pT < " + std::to_string(pTMax) + " GeV"),
  fpTMin(pTMin),
  fpTMax(pTMax)
    {
    }

  private:
    double fpTMax; //Maximum pT allowed in GeV/c
    double fpTMin; // Min pT allowed in GeV/c
    bool checkConstraint(const UNIVERSE& univ) const //override
    {
      double truepT = univ.GetMuonPTTrue()/1000.;
      return truepT > fpTMin && truepT < fpTMax;
    }
};
