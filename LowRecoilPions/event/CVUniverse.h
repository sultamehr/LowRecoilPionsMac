// =============================================================================
// Base class for an un-systematically shifted (i.e. CV) universe. Implement
// "Get" functions for all the quantities that you need for your analysis.
//
// This class inherits from PU::MinervaUniverse, which in turn inherits from
// PU::BaseUniverse. PU::BU defines the interface with anatuples.
// 
// Within the class, "WeightFunctions" and "MuonFunctions" are included to gain
// access to standardized weight and muon variable getters. See:
// https://cdcvs.fnal.gov/redmine/projects/minerva-sw/wiki/MinervaUniverse_Structure_
// for a full list of standardized functions you can use. In general, if a
// standard version of a function is available, you should be using it.
// =============================================================================
#ifndef CVUNIVERSE_H
#define CVUNIVERSE_H

#include <iostream>

#include "PlotUtils/MinervaUniverse.h"
//#include "event/Cluster.h"
class CVUniverse : public PlotUtils::MinervaUniverse {

  public:
  #include "PlotUtils/MuonFunctions.h" // GetMinosEfficiencyWeight
  #include "PlotUtils/TruthFunctions.h" //Getq3True
  #include "PlotUtils/LowRecoilFunctions.h" // GetEAvailable()
  #include "PlotUtils/WeightFunctions.h" // Get COH Weight functions  
  // ========================================================================
  // Constructor/Destructor
  // ========================================================================
  CVUniverse(PlotUtils::ChainWrapper* chw, double nsigma = 0)
      : PlotUtils::MinervaUniverse(chw, nsigma) {}

  virtual ~CVUniverse() {}

  // ========================================================================
  // Quantities defined here as constants for the sake of below. Definition
  // matched to Dan's CCQENuInclusiveME variables from:
  // `/minerva/app/users/drut1186/cmtuser/Minerva_v22r1p1_OrigCCQENuInc/Ana/CCQENu/ana_common/include/CCQENuUtils.h`
  // ========================================================================
  static constexpr double M_n = 939.56536;
  static constexpr double M_p = 938.272013;
  static constexpr double M_nucleon = (1.5*M_n+M_p)/2.5;
  static constexpr double M_mu = 105.6583/1000.; //Converting to GeV
  static constexpr int PDG_n = 2112;
  static constexpr int PDG_p = 2212;
  static constexpr double M_pi = 139.57039; //in MeV
  // ========================================================================
  // Write a "Get" function for all quantities access by your analysis.
  // For composite quantities (e.g. Enu) use a calculator function.
  //
  // In order to properly calculate muon variables and systematics use the
  // various functions defined in MinervaUniverse.
  // E.g. GetPmu, GetEmu, etc.
  // ========================================================================

  // Quantities only needed for cuts
  // Although unlikely, in principle these quanties could be shifted by a
  // systematic. And when they are, they'll only be shifted correctly if we
  // write these accessor functions.
  double GetTrueQ3() const
  {
 	return Getq3True()/pow(10,3);
  } 
  
  double GetTrueQ2GeV() const
  {
        return GetQ2True()/pow(10, 6);
  }

  double GetTrueQ2MeV() const
  {
        return GetQ2True();
  }

  double GetTrueQ0GeV() const
  {
        return Getq0True()/pow(10,3);
  }
  double GetTrueQ0() const
  {     
        return Getq0True();
  }

  //Muon kinematics
  double GetMuonPT() const //GeV/c
  {
    return GetPmu()/1000. * sin(GetThetamu());
  }

  double GetMuonPz() const //GeV/c
  {
    return GetPmu()/1000. * cos(GetThetamu());
  }

  double GetMuonP() const
  {
	return GetPmu()/1000.; //GeV/c
  }
  double GetPmuTrue() const
  {
	return GetPlepTrue()/1000.; //GeV/c

  }
  double GetTrueExpq3() const
  {
     return calcq3(GetTrueExperimentersQ2(), GetEnuTrue()/pow(10,3), GetElepTrue()/pow(10,3)); // In GeV
  }
  double GetMuonPTTrue() const //GeV/c
  {
    return GetPlepTrue()/1000. * sin(GetThetalepTrue());
  }

  double GetMuonPzTrue() const //GeV/c
  {
    return GetPlepTrue()/1000. * cos(GetThetalepTrue());
  }

  double GetEmuGeV() const //GeV
  {
    return GetEmu()/1000.;
  }

  double GetElepTrueGeV() const //GeV
  {
    return GetElepTrue()/1000.;
  }

  int GetInteractionType() const {
    return GetInt("mc_intType");
  }

  int GetTargetNucleon() const {
    return GetInt("mc_targetNucleon");
  }
  
  double GetBjorkenXTrue() const {
    return GetDouble("mc_Bjorkenx");
  }
  double GetZero() const{
    return 0.0;
  }

  double GetBjorkenYTrue() const {
    return GetDouble("mc_Bjorkeny");
  }

  virtual bool IsMinosMatchMuon() const {
    bool ismatch = GetBool("isMinosMatchTrack");
    return ismatch;
  }
 
  virtual bool isMuonCharge() const{
   double charge = GetDouble("MasterAnaDev_minos_trk_qp");
   return charge < 0.0;
  }
  
  ROOT::Math::XYZTVector GetVertex() const
  {
    ROOT::Math::XYZTVector result;
    result.SetCoordinates(GetVec<double>("vtx").data());
    return result;
  }

  ROOT::Math::XYZTVector GetTrueVertex() const
  {
    ROOT::Math::XYZTVector result;
    result.SetCoordinates(GetVec<double>("mc_vtx").data());
    return result;
  }

  virtual int GetTDead() const {
    return GetInt("phys_n_dead_discr_pair_upstream_prim_track_proj");
  }
  
  //TODO: If there was a spline correcting Eavail, it might not really be Eavail.
  //      Our energy correction spline, one of at least 2 I know of, corrects q0
  //      so that we get the right neutrino energy in an inclusive sample.  So,
  //      this function could be correcting for neutron energy which Eavail should
  //      not do.
  
  double GetEavailCorr() const {
       return GetEAvailable();
  }  

  virtual double GetEavail() const {
    //if (GetEAvailable() > 1500.) PrintArachneLink(); 
    return GetEAvailable();
    //return eavail;
    //return GetEAvailable();
   }
 
  virtual double GetNoPolylineE() const{
	return GetDouble("recoil_E_nopolyline");
  }
  virtual double GetTrueEHad() const{
    return GetEnuTrue() - GetElepTrue();

  }
  virtual double GetPolylineE() const {
	return GetDouble("recoil_E_polylinecorrected");	
  }
 
  virtual double GetRecoilECorr() const {
    double eavail = GetEAvailable() + M_pi;///GetDouble("recoil_E_nopolyline");
    return eavail;
    //if (eavail < 400.) return 50.+1.7*eavail;
    //if (eavail > 400.) return 1.7*eavail;
  } 
  virtual double GetRecoilE() const {
    double eavail = GetEAvailable()+M_pi; //GetDouble("recoil_E_nopolyline");
    return eavail;
  }  

  virtual double GetEHad() const {
     return GetCalRecoilE();
  }

  virtual double GetCalRecoilE() const{
    const double calE = GetDouble("MasterAnaDev_hadron_recoil_default");
    if (calE > 1000.){
        return GetCalRecoilE_CCPiSPline();
    }
    else {
        return GetCalRecoilE_Corrected();
    }

  }

  virtual double GetCalRecoilE_CCPiSPline() const{
    return GetDouble("MasterAnaDev_hadron_recoil_two_track");

  }

  virtual double GetCalRecoilE_Corrected() const {
    double calE = GetDouble("MasterAnaDev_hadron_recoil_default");
    double calE_corrected = calE; //make correction to this
    std::vector<double> calE_bins = {0.0, 0.025e3, 0.05e3, 0.075e3, 0.1e3, 0.15e3, 0.2e3, 0.25e3, 0.3e3, 0.4e3, 0.5e3,0.6e3, 0.7e3,   0.8e3,  0.9e3,   1.0e3, 2.0e3};
    const std::vector<double> corrections = {-0.060, -0.050, -0.210, -0.180,
                                           -0.165, -0.180, -0.180, -0.180,
                                           -0.195, -0.360, -0.400};
    for (int i_bin = 0; i_bin < calE_bins.size() - 1; ++i_bin) {
      if (((1e3) * calE_bins[i_bin] < calE) &&
        (calE < (1e3) * calE_bins[i_bin + 1])) {
        if (calE < 1000.) {
          calE_corrected = calE - (1e3) * corrections[i_bin];
          break;
        }
      }
     }
    return calE_corrected;
  } 


  virtual double Recoq3pTdiff() const{
     double q3 = Getq3();
     double pT = GetMuonPT(); 
     return sqrt( (q3*q3) - (pT*pT));

  }

  virtual double GetTrueq3pTdiff() const {
     double q3 = GetTrueQ3();
     double pT =  GetMuonPTTrue(); 
     return sqrt( (q3*q3) - (pT*pT));

  }
  /*
  virtual double GetRecoilESum() const {
     double ecal = GetDouble("blob_recoil_E_ecal");
     double hcal = GetDouble("blob_recoil_E_hcal");
     double tracker = GetDouble("blob_recoil_E_tracker");
     double od = GetDouble("blob_recoil_E_od");
     return (ecal+hcal+tracker); //Get these in GeV
  }

  virtual double GetRecoilECorrSum() const {
     //These correction values are obtained from docdb 7712
     double ecal = 1.73*GetDouble("blob_recoil_E_ecal"); // Adding calorimetric correction for ECAL (not doing side ecal)
     double hcal = 9.32*GetDouble("blob_recoil_E_hcal"); //Calorimetric correction for HCAL
     double tracker = GetDouble("blob_recoil_E_tracker"); // Calorimetric correction for tracker
     double od = 9.32*GetDouble("blob_recoil_E_od"); // Calorimetric correction for OD
     return 1.70*(ecal+hcal+tracker);
    
  }
  */
  virtual double GetRecoilESumPi() const {
     double ecal = GetDouble("blob_recoil_E_ecal");
     double hcal = GetDouble("blob_recoil_E_hcal");
     double tracker = GetDouble("blob_recoil_E_tracker");
     double od = GetDouble("blob_recoil_E_od");
     return (ecal+hcal+tracker)/pow(10,3); //Get these in GeV
  }

  virtual double GetQ2Reco() const{
    //double q2reco = GetDouble("MasterAnaDev_Q2_Inclusive")/pow(10,6);
    
    double enu = GetEmuGeV() + NewRecoilE()/1000.;
    double pmucos = (GetPmu()/1000.)*cos(GetThetamu());
    double q2reco = 2.*enu*(GetEmuGeV() - pmucos ) - (M_mu*M_mu);
    //std::cout << "Print RecoilSummedE" << GetRecolE() <<  " Print Ehad: " << GetEHad() << " Print q2reco: " << q2reco << " Print EAvail: " << GetEavail() << std::endl;
    //double q2reco = GetDouble("qsquared_recoil");
    
    return  q2reco; // TODO: July 20, 2022 Check Using Hang Su's Low recoil function; //q2reco;
  }
  
  virtual double GetQ2RecoMeV() const{
    return GetQ2Reco()*pow(10,6);

  }

  virtual double GetCOHPionWeight() const {
     if(GetInt("mc_intType") == 4){
       int npi = GetTrueNPionsinEvent();
       if (npi == 0) return 1.0;
       double angle = GetTrueAngleHighTpi();
       double KE = GetTrueHighTpi();
       double weight = GetCoherentPiWeight(angle, KE);
       return weight;
     }
     else return 1.0;
  }

   virtual double GetTrueAngleHighTpi() const {
     int nFSpi = GetTrueNPions();
     double angle = -9999.; //WRTbeam and in degrees
     double pionKE = 0.0;
     int idk = -9999;
     for (int i = 0; i < nFSpi; i++){
         int pdg = GetVecElem("mc_FSPartPDG",i);
         if(pdg != 211) continue;
         double energy = GetVecElem("mc_FSPartE", i);
         double mass = 139.569;
         double tpi = energy - mass;
         if (tpi >= pionKE) {
               pionKE = tpi;
               TVector3 pimomentumvec(GetVecElem("mc_FSPartPx", i), GetVecElem("mc_FSPartPz", i),GetVecElem("mc_FSPartPz", i));
               double deg_wrtb = thetaWRTBeam(pimomentumvec.X(), pimomentumvec.Y(), pimomentumvec.Z()); //rad
               angle = deg_wrtb*180./M_PI;
         }
      }

      return angle;
   }
  
   virtual double GetTrueHighTpi() const {
     int nFSpi = GetTrueNPions();
     double pionKE = 0.0;
     for (int i = 0; i < nFSpi; i++){
          int pdg = GetVecElem("mc_FSPartPDG",i);
          if(pdg != 211) continue;
          double energy = GetVecElem("mc_FSPartE", i);
          double mass = 139.569;
          double tpi = energy - mass;
          if (tpi >= pionKE) pionKE = tpi;
      }

      return pionKE;
   }


  //GetRecoilE is designed to match the NSF validation suite
  //virtual double GetRecoilE() const {
  //  return GetVecElem("recoil_summed_energy", 0);
  //}
  
  virtual double Getq3() const{
    double eavail = NewRecoilE()/1000.;
    double q2 = GetQ2Reco();
    //std::cout << " Dan's EHad is " << eavail << " Q2 is " << q2 << std::endl;
    double q3mec = sqrt(eavail*eavail + q2);
    //std::cout << "Print TrueEavail: " << GetTrueEAvail() <<  " Print RecoilE: " << GetRecoilE() << " Print q2reco: " << GetQ2Reco() << " Print EAvail: " << GetEavail() << " Print q3: " << q3mec  << std::endl;
    return  q3mec; // Using Hang's q3 definition TODO: check to see if using Hang's definition is ok. //q3mec;
  }
   
  virtual int GetCurrent() const { return GetInt("mc_current"); }

  virtual int GetTruthNuPDG() const { return GetInt("mc_incoming"); }

  virtual double GetMuonQP() const {
    return GetDouble((GetAnaToolName() + "_minos_trk_qp").c_str());
  }

  //Some functions to match CCQENuInclusive treatment of DIS weighting. Name matches same Dan area as before.
  virtual double GetTrueExperimentersQ2() const {
    double Enu = GetEnuTrue()/pow(10,3); //GeV
    double Emu = GetElepTrue()/pow(10,3); //GeV
    double thetaMu = GetThetalepTrue();
    return 4.0*Enu*Emu*pow(sin(thetaMu/2.0),2.0);//GeV^2
  }

  virtual double CalcTrueExperimentersQ2(double Enu, double Emu, double thetaMu) const{
    return 4.0*Enu*Emu*pow(sin(thetaMu/2.0),2.0);//MeV^2
  }

  virtual double GetTrueExperimentersW() const {
    double nuclMass = M_nucleon;
    int struckNucl = GetTargetNucleon();
    if (struckNucl == PDG_n){
      nuclMass=M_n;
    }
    else if (struckNucl == PDG_p){
      nuclMass=M_p;
    }
    double Enu = GetEnuTrue();
    double Emu = GetElepTrue();
    double thetaMu = GetThetalepTrue();
    double Q2 = CalcTrueExperimentersQ2(Enu, Emu, thetaMu);
    return TMath::Sqrt(pow(nuclMass,2) + 2.0*(Enu-Emu)*nuclMass - Q2);
  }

  // Adding Branches for Mehreen's Michels
  virtual int GetNMichels() const{
      return GetInt("FittedMichel_michel_fitPass_sz");
  }

  virtual std::vector<int> GetTrueFSPDGCodes() const {
     std::vector<int> pdgcodes;
     int pdgsize = GetInt("mc_nFSPart");
     for (int i = 0; i< pdgsize; i++)
      {
          int pdg = GetVecElem("mc_FSPartPDG", i);
          pdgcodes.push_back(pdg);
      }

      return pdgcodes;
  }       
  
  virtual int GetTrueNMichels() const {
     int ntruemichels = 0;
     int nmichels = GetNMichels();
     for (int i =0; i< nmichels; i++)
     {
       
      double datafrac =  GetVecElem("FittedMichel_michel_datafraction", i);
      int fitted = GetVecElem("FittedMichel_michel_fitPass", i);
      if (fitted == 0) continue;
      if (datafrac > 0.5) continue;
      ntruemichels++;

     }

     return ntruemichels;
  }

 virtual bool GetTrueIsFSPartInEvent(const int pdg) const {
     std::vector<int> FSparts = GetTrueFSPDGCodes();
     std::vector<int>::iterator it;
     it = std::find(FSparts.begin(), FSparts.end(), pdg);
     if (it != FSparts.end())
     {
	return true;
     }
     else return false;

 }

 virtual double GetTrueWexp() const {
    double  W = pow(M_p, 2.0) -  GetTrueQ2MeV() +
             2.0 * (M_p)*GetTrueEHad();

    W = W > 0 ? sqrt(W) : 0.0;
    return W/1000.;

  }

 virtual double GetWExp() const {
    double W = pow(M_p, 2.0) - GetQ2RecoMeV() +
             2.0 * (M_p)*GetEHad();
    W = W > 0 ? sqrt(W) : 0.0;
    return W/1000.;
  }

 

 
 virtual double GetTrueIntVtxX() const {return GetVecElem("mc_vtx", 0);}
 virtual double GetTrueIntVtxY() const {return GetVecElem("mc_vtx", 1);}
 virtual double GetTrueIntVtxZ() const {return GetVecElem("mc_vtx", 2);}
 
 virtual bool IsInHexagon(double x, double y, double apothem) const {
   double lenOfSide = apothem * (2 / sqrt(3));
   double slope = (lenOfSide / 2.0) / apothem;
   double xp = fabs(x);
   double yp = fabs(y);

   if ((xp * xp + yp * yp) < apothem * apothem)
       return true;
   else if (xp <= apothem && yp * yp < lenOfSide / 2.0)
       return true;
   else if (xp <= apothem && yp < lenOfSide - xp * slope)
       return true;

   return false;
 }

 virtual bool IsInPlastic() const {
    if (!IsInHexagon(GetVecElem("mc_vtx", 0), GetVecElem("mc_vtx", 1), 1000.0)) return false;  // This is in the calorimeters
    double mc_vtx_z = GetVecElem("mc_vtx", 2);
    if (mc_vtx_z > 8467.0) return false;  // Ditto
    int mc_nuclei = GetInt("mc_targetZ");
    // In the carbon target?  The z is gotten from NukeBinningUtils
    if (fabs(mc_vtx_z - 4945.92) <=
          PlotUtils::TargetProp::ThicknessMC::Tgt3::C / 2 &&
      mc_nuclei == 6)
    return false;
    // Is it in water Targer?
    if (5200 < mc_vtx_z && mc_vtx_z < 5420 && (mc_nuclei == 8 || mc_nuclei == 1))
    return false;
    // Finally, do you have target material?  I'm going to say lead/iron isn't a
    //   // big consideration elsewhere in the detector
    if (mc_nuclei == 26 || mc_nuclei == 82) return false;
    return true; 
 }
 
 virtual double GetDiffractiveWeight() const {
    if (GetInt("mc_intType") != 4) return 1.;
    if (!IsInPlastic() && !PlotUtils::TargetUtils::Get().InWaterTargetMC(
                            GetTrueIntVtxX(), GetTrueIntVtxY(),
                            GetTrueIntVtxZ(), GetInt("mc_targetZ"))) {
    return 1.;
    }
    return 1.4368;
 }

 virtual int GetTrueNPionsinEvent() const {
     int npion = 0;
     
     int pdgsize = GetInt("mc_nFSPart");
     for (int i = 0; i< pdgsize; i++)
      {
          int pdg = GetVecElem("mc_FSPartPDG", i);
          if (pdg == 211) npion++;
      }
      return npion;

 }
 
 virtual int GetTrueNKaonsinEvent() const {
     int npart = 0;
     int pdgsize = GetInt("mc_nFSPart");
     for (int i = 0; i< pdgsize; i++)
      {
          int pdg = GetVecElem("mc_FSPartPDG", i);
          if (pdg == 321) npart++;
      }
      return npart;
 }

 virtual int GetTrueNPi0inEvent() const {
     int npart = 0;
     int pdgsize = GetInt("mc_nFSPart");
     for (int i = 0; i< pdgsize; i++)
      {
          int pdg = GetVecElem("mc_FSPartPDG", i);
          if (pdg == 111) npart++;
      } 
      return npart;
 }



 virtual double GetTrueLowestTpiEvent() const {
    double tpi = 999999.;
    int pdgsize = GetInt("mc_nFSPart");
    for (int i = 0; i< pdgsize; i++)
        {
            int pdg = GetVecElem("mc_FSPartPDG", i);
            if (pdg != 211) continue;
	    double energy = GetVecElem("mc_FSPartE", i);
	    double momentumx = GetVecElem("mc_FSPartPx", i);
	    double momentumy = GetVecElem("mc_FSPartPy", i);
	    double momentumz = GetVecElem("mc_FSPartPz", i);
            double pionmomentum = TMath::Sqrt(pow(momentumx, 2) + pow(momentumy,2)+pow(momentumz,2));
	   // ROOT::Math::PxPyPzEVector pion;
	   // pion.SetPxPyPzE(momentumx, momentumy,momentumz, energy); 
           // double momentum = pion.P();
	    double pionmass = TMath::Sqrt(pow(energy, 2) - pow(pionmomentum, 2));  
	    double KE = energy - pionmass;
	    if (tpi > KE) tpi = KE;     
        }
    ///if(tpi < 99999.) std::cout << "Lowest Energy Primary Pion KE is " << tpi << std::endl;
    return tpi;
 } 
  
  virtual int GetTrueNPions() const{
      return GetInt("truth_FittedMichel_all_piontrajectory_pdg_sz");
  }

  virtual int GetTrueNPiPlus() const{
	int npiplus = 0;
	int npi = GetTrueNPions();
	for (int i = 0; i < npi; i++){
	    int pdg = GetVecElem("truth_FittedMichel_all_piontrajectory_pdg", i);
	    if (pdg == 211) npiplus++;
	
	}
	return npiplus;
  }

  virtual int GetPionParentID(int i) const {
     return GetVecElem("truth_FittedMichel_all_piontrajectory_ParentID", i);
  }

  virtual int GetPionPDG(int i) const{
     return GetVecElem("truth_FittedMichel_all_piontrajectory_pdg", i);
  }

  virtual double GetPionE(int i) const{
     return GetVecElem("truth_FittedMichel_all_piontrajectory_energy",i)/pow(10,3);
  }

  virtual double GetPionP(int i) const{
    return GetVecElem("truth_FittedMichel_all_piontrajectory_momentum", i)/pow(10,3);
  }

  virtual double GetPionMass(int i) const{
    double pionmass = sqrt(pow(GetPionE(i), 2) - pow(GetPionP(i), 2));
    return pionmass;
  }

  virtual double GetPionKE(int i) const{
    double pionKE = GetPionE(i) - GetPionMass(i);
  }

  virtual double thetaXWRTBeam(double x, double y, double z) const{

  double pzp = cos( MinervaUnits::numi_beam_angle_rad)*z + sin( MinervaUnits::numi_beam_angle_rad)*y;
  int sign = (x<0.) ? -1 : 1;
  double denom2 = pow(x,2) + pow(pzp,2);
  if( 0. == denom2 ) return -999.;
  else
    return sign * acos(pzp / sqrt(denom2));
  }

  virtual double thetaWRTBeam(double x, double y, double z) const{
      double pyp = -1.0 *sin( MinervaUnits::numi_beam_angle_rad)*z + cos( MinervaUnits::numi_beam_angle_rad )*y;
      double pzp = cos( MinervaUnits::numi_beam_angle_rad )*z + sin( MinervaUnits::numi_beam_angle_rad )*y;
      double denom2 = pow(x,2) + pow(pyp,2) + pow(pzp,2);
      if( 0. == denom2 ) return -9999.;
      else return acos(pzp / sqrt(denom2) );
  } 

  virtual double GetTrueTpi() const {
     int nFSpi = GetTrueNPions();
     double pionKE = 9999.;
     for (int i = 0; i < nFSpi; i++){
         int pdg = GetVecElem("truth_FittedMichel_all_piontrajectory_pdg", i);
         int pitrackid = GetVecElem("truth_FittedMichel_all_piontrajectory_ParentID", i);

         double energy = GetVecElem("truth_FittedMichel_all_piontrajectory_energy",i);
         double p = GetVecElem("truth_FittedMichel_all_piontrajectory_momentum", i);
         double mass = sqrt(pow(energy,2) - pow(p, 2));
         double tpi = energy - mass;
         if (tpi <= pionKE) pionKE = tpi;
      }
       
      return pionKE;
   }
 virtual void PrintTrueArachneLink() const {
  int link_size = 200;
  char link[link_size];
  int run = GetInt("mc_run");
  int subrun = GetInt("mc_subrun");
  int gate = GetInt("mc_nthEvtInFile") + 1;
  int slice = GetVecElem("slice_numbers", 0);
  sprintf(link,
          "https://minerva05.fnal.gov/Arachne/"
          "arachne.html\?det=SIM_minerva&recoVer=v21r1p1&run=%d&subrun=%d&gate="
          "%d&slice=%d",
          run, subrun, gate, slice);
       	  std::cout << link << std::endl;
       	  std::cout << "Lepton E: " <<  GetElepTrueGeV() << " Run " << run << "/"<<subrun << "/" << gate << "/" << slice << std::endl;
	  std::cout << "Printing Available Energy " << NewEavail() << std::endl;
	  std::cout << "Muon P: " << GetMuonP() << std::endl;
	  std::cout << "Get Muon Pt: " << GetMuonPT() << std::endl;
	  std::cout << "Get Muon Pz: " << GetMuonPz() << std::endl;
	  std::cout << "Get Muon PT True " << GetMuonPTTrue() << std::endl;
 }

 virtual void PrintDataArachneLink() const {
  int link_size = 200;
  char link[link_size];
  int run = GetInt("ev_run");
  int subrun = GetInt("ev_subrun");
  int gate = GetInt("ev_gate");
  int slice = GetVecElem("slice_numbers", 0);
  sprintf(link,
          "https://minerva05.fnal.gov/Arachne/"
          "arachne.html\?det=MV&recoVer=v21r1p1&run=%d&subrun=%d&gate="
          "%d&slice=%d",
          run, subrun, gate, slice);
       if(NewEavail() < 50. && GetMuonPT() < .20){
        std::cout << link << std::endl;
       // std::cout << "Lepton E: " <<  GetMuonPT() << " Run " << run << "/"<<subrun << "/" << gate << "/" << slice << std::endl;
       }

 }
 

 virtual double GetClusterEnergyTracker() const
 {
    int nclusters = GetInt("cluster_view_sz");
    double totenergy = GetDouble("cluster_trackerEsum"); //0.0;
    /*
    for (int i = 0; i < nclusters; i++){
	int ismuon = GetVecElemInt("cluster_isMuontrack", i);
	if (ismuon != 0) continue;
    	double time = GetVecElem("cluster_time", i); 
        double vtxtime = GetVertex().T(); //mus
    	double timediff = vtxtime - time;
    	//int ismuon = GetVecElemInt("cluster_isMuontrack", i);
    	double ecal = GetVecElem("cluster_ecalo", i);
	int subdet = GetVecElem("cluster_subdet", i);
	if (subdet != 2) continue; 
   	//if (ismuon !=0) continue; // check to make sure cluster is not on muon track, 0 is not muon, 1 is muon
        if (timediff < -20. || timediff > 30.) continue;
        totenergy += ecal;     
    }
    */
    return totenergy;
 }

 virtual double GetClusterEnergyECAL() const
 {
    double totenergy = GetDouble("cluster_ecalEsum");//0.0;
    int nclusters = GetInt("cluster_view_sz"); 
    /*
    for (int i = 0; i < nclusters; i++){
 	int ismuon = GetVecElemInt("cluster_isMuontrack", i);
	if (ismuon != 0) continue;
        double time = GetVecElem("cluster_time", i); //mus
        double vtxtime = GetVertex().T(); //mus
        double timediff = vtxtime - time;
        //int ismuon = GetVecElemInt("cluster_isMuontrack", i);
        double ecal = GetVecElem("cluster_ecalo", i);
        int subdet = GetVecElem("cluster_subdet", i);
        if (timediff < -20. || timediff > 30.) continue;
        if (subdet != 3) continue;
        //if (ismuon !=0) continue; // check to make sure cluster is not on muon track, 0 is not muon, 1 is muon
        totenergy += ecal;
    }
    */
    return totenergy;
 }
 
 virtual double GetTrackerMuFuzz() const
 {
    double mufuzz = 0;
    int nfuzz = GetInt("muon_fuzz_per_plane_r80_planeIDs_sz");
    for (int i =0; i < nfuzz; i++){
	int planeID = GetVecElem("muon_fuzz_per_plane_r80_planeIDs", i);
	double fuzze = GetVecElem("muon_fuzz_per_plane_r80_energies", i);
 	if (planeID < 1504968704 || planeID > 1560805376) continue; //sum fuzz for only in Tracker
	mufuzz += fuzze;
    }
    return mufuzz;
 } 

 virtual double GetECALMuFuzz() const
 {
    double mufuzz = 0;
    int nfuzz = GetInt("muon_fuzz_per_plane_r80_planeIDs_sz");
    for (int i =0; i < nfuzz; i++){
        int planeID = GetVecElem("muon_fuzz_per_plane_r80_planeIDs", i);
        double fuzze = GetVecElem("muon_fuzz_per_plane_r80_energies", i);
        if (planeID < 1700003840 || planeID > 1709703168) continue; //sum fuzz for only in ECAL
        mufuzz += fuzze;
    }
    return mufuzz;
 }

 virtual double NewEavail() const
 {
    double recoiltracker =  GetDouble("blob_recoil_E_tracker") -  GetTrackerMuFuzz(); 
    //double recoiltracker = GetClusterEnergyTracker() - GetTrackerMuFuzz();
    double recoilEcal = GetDouble("blob_recoil_E_ecal") - GetECALMuFuzz();
    //double recoilEcal = GetClusterEnergyECAL() - GetECALMuFuzz();
    const double Eavailable_scale = 1.17;
    double eavail = recoiltracker + recoilEcal;
    return eavail*Eavailable_scale;
 }

 virtual double NewRecoilE() const
 {

    double recoil = NewEavail() + M_pi;
    double newrecoil = recoil;
    //if (recoil > 75. and recoil < 300.) newrecoil = recoil - 75.;
    //else if(recoil > 300. and recoil < 600.) newrecoil = recoil - 25.;
    //else if(recoil > 600. and recoil < 1000.) newrecoil = recoil + 25.;
    //else if(recoil > 1000.) newrecoil = recoil + 175.;
    return  newrecoil;

 } 
 


 virtual double GetTrueEAvail() const
 {
     double Eavail = 0.0;
     int pdgsize = GetInt("mc_nFSPart");
     for (int i = 0; i< pdgsize; i++)
     {
        int pdg = GetVecElem("mc_FSPartPDG", i);  
        double energy = GetVecElem("mc_FSPartE", i); // hopefully this is in MeV
        /* 
        if(abs(pdg) == 211){ // charged pions (kinetic energy)
          double kinetic_energy = energy - M_pi;
          Eavail += kinetic_energy;
        } 
        else if (abs(pdg) == 311 or pdg == 321){
	  // Add Kaons
	  double KE = energy - 497.611/2.0;
	  Eavail += KE;
        }
        else if (pdg == 2212){ // protons (kinetic energy)
          double kinetic_energy = energy - M_p;
	  Eavail += kinetic_energy;
        }
        else if ((pdg == 111) or (abs(pdg) == 11) or (pdg == 22)){
	  // neutral pions, photons, electrons and positrons (total energy)
	  Eavail += energy;
      	} 
        else if (pdg > 1000000000){
          // do nothing for nucleons, assume negligible,
          Eavail += 0;
        }
        else if (pdg >= 2000){
 	 //Strange Baryons
         double kinetic_energy = energy - M_p;
	 Eavail += kinetic_energy;
        }
        else if (pdg <= -2000){
	 // antibaryons, primarily -2112 and -2212s
	 Eavail += energy + M_p;
	}
        else { // Get any other particle. strange mesons?
   	 Eavail += energy;
        }
        */	  
        if (pdg == 211 || pdg == -211) Eavail+= energy - 139.57; // subtracting pion mass to get Kinetic energy
        else if (pdg == 2212) Eavail += energy - 938.28; // proton
        else if (pdg == 111) Eavail += energy; // pi0
        else if (pdg == 22) Eavail += energy; // photons
        else if (pdg == 311) Eavail += energy - 497.611/2.0; // K0 ???? - Kaon rest mass / 2
        else if (abs(pdg) == 321) Eavail += energy - 497.611/2.0; // Kaon+ Kinetic Energy  divide by Kmass/2 
        else if (pdg == 221) Eavail += energy; //Adding etas
	else if (pdg == 3222) Eavail += energy - 1115.683; // mass of Lambda 
	else if (pdg == 3122) Eavail += energy - 1189.37 ; // mass of Sigma
        	
        //else if (abs(pdg) !=  2112) Eavail += energy; //Adding anything else except neutrons.  
       
     }
     //std::cout << "True Eavail is " << Eavail << " --- Reco Eavail is " << GetEavail() << "New EAvail is " << GetNewEavail() << std::endl;
     return Eavail; // in MeV
 }


 


  //Still needed for some systematics to compile, but shouldn't be used for reweighting anymore.
  protected:
  #include "PlotUtils/WeightFunctions.h" // Get*Weight
};

#endif
