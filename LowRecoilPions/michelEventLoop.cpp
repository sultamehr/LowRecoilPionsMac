#define MC_OUT_FILE_NAME "runEventLoopMC_July202022_EavailComp_q3bin6_reweight.root"
#define DATA_OUT_FILE_NAME "runEventLoopData_July202022_EavailComp_q3bin6_reweight.root"
#define MC_SIDE_FILE_NAME "runEventLoopMC_July202022_Sideband_q3bin6_reweight.root"
#define DATA_SIDE_FILE_NAME "runEventLoopDATA_July202022_Sideband_q3bin6_reweight.root"


#define USAGE \
"\n*** USAGE ***\n"\
"runEventLoop <dataPlaylist.txt> <mcPlaylist.txt>\n\n"\
"*** Explanation ***\n"\
"Reduce MasterAnaDev AnaTuples to event selection histograms to extract a\n"\
"single-differential inclusive cross section for the 2021 MINERvA 101 tutorial.\n\n"\
"*** The Input Files ***\n"\
"Playlist files are plaintext files with 1 file name per line.  Filenames may be\n"\
"xrootd URLs or refer to the local filesystem.  The first playlist file's\n"\
"entries will be treated like data, and the second playlist's entries must\n"\
"have the \"Truth\" tree to use for calculating the efficiency denominator.\n\n"\
"*** Output ***\n"\
"Produces a two files, " MC_OUT_FILE_NAME " and " DATA_OUT_FILE_NAME ", with\n"\
"all histograms needed for the ExtractCrossSection program also built by this\n"\
"package.  You'll need a .rootlogon.C that loads ROOT object definitions from\n"\
"PlotUtils to access systematics information from these files.\n\n"\
"*** Environment Variables ***\n"\
"Setting up this package appends to PATH and LD_LIBRARY_PATH.  PLOTUTILSROOT,\n"\
"MPARAMFILESROOT, and MPARAMFILES must be set according to the setup scripts in\n"\
"those packages for systematics and flux reweighters to function.\n"\
"If MNV101_SKIP_SYST is defined at all, output histograms will have no error bands.\n"\
"This is useful for debugging the CV and running warping studies.\n\n"\
"*** Return Codes ***\n"\
"0 indicates success.  All histograms are valid only in this case.  Any other\n"\
"return code indicates that histograms should not be used.  Error messages\n"\
"about what went wrong will be printed to stderr.  So, they'll end up in your\n"\
"terminal, but you can separate them from everything else with something like:\n"\
"\"runEventLoop data.txt mc.txt 2> errors.txt\"\n"

enum ErrorCodes
{
  success = 0,
  badCmdLine = 1,
  badInputFile = 2,
  badFileRead = 3,
  badOutputFile = 4
};

//PlotUtils includes
//No junk from PlotUtils please!  I already
//know that MnvH1D does horrible horrible things.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

//Includes from this package
#include "event/CVUniverse.h"
#include "event/MichelEvent.h"
#include "systematics/Systematics.h"
#include "cuts/MaxPzMu.h"
#include "util/Variable.h"
#include "util/Variable2D.h"
#include "util/GetFluxIntegral.h"
#include "util/GetPlaylist.h"
#include "cuts/SignalDefinition.h"
//#include "util/TruthInteractionStudies.h"
#include "util/PionFSCategory.h"
#include "cuts/q3RecoCut.h"
#include "studies/Study.h"
//#include "studies/PerMichelVarByGENIELabel.h"
#include "studies/PerMichelEventVarByGENIELabel.h"  
#include "studies/PerMichel2DVar.h"
#include "studies/PerMichel2DVarbin.h"
#include "studies/PerMichelVarVec.h"
#include "studies/PerMichelEvent2DVarBin.h"
#include "studies/PerEventVarBin.h"
#include "studies/PerMichelVarVecFSPart.h"
#include "cuts/hasMichel.h"
#include "event/Michel.h"
#include "cuts/BestMichelDistance2D.h"
#include "cuts/VtxMatchFirst.h"
#include "cuts/hasTruePion.h"
#include "cuts/PTRangeReco.h"
#include "cuts/GetClosestMichel.h"
#include "cuts/Distance2DSideband.h"
#include "event/SetDistanceMichelSideband.h"
#include "event/SetDistanceMichelSelection.h"
#include "event/GetClosestMichel.h"
//#include "Binning.h" //TODO: Fix me

//PlotUtils includes
#include "PlotUtils/makeChainWrapper.h"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#include "PlotUtils/MacroUtil.h"
#include "PlotUtils/MnvPlotter.h"
#include "PlotUtils/CCInclusiveCuts.h"
#include "PlotUtils/CCInclusiveSignal.h"
#include "PlotUtils/CrashOnROOTMessage.h" //Sets up ROOT's debug callbacks by itself
#include "PlotUtils/Cutter.h"
#include "PlotUtils/Model.h"
#include "PlotUtils/FluxAndCVReweighter.h"
#include "PlotUtils/GENIEReweighter.h"
#include "PlotUtils/LowRecoil2p2hReweighter.h"
#include "PlotUtils/RPAReweighter.h"
#include "PlotUtils/MINOSEfficiencyReweighter.h"
#include "PlotUtils/TargetUtils.h"
#include "../MAT-MINERvA/weighters/PionReweighter.h"
#include "PlotUtils/LowRecPionSignal.h"
#pragma GCC diagnostic pop

//ROOT includes
#include "TParameter.h"

//c++ includes
#include <iostream>
#include <cstdlib> //getenv()

//==============================================================================
// Loop and Fill
//==============================================================================
void LoopAndFillEventSelection(
    PlotUtils::ChainWrapper* chain,
    std::map<std::string, std::vector<CVUniverse*> > error_bands,
    std::vector<Variable*> vars,
    std::vector<Variable2D*> vars2D,
    std::vector<Study*> studies,
    std::vector<Variable*> sidevars,
    std::vector<Variable2D*> sidevars2D,
    std::vector<Study*> sideband_studies,
    PlotUtils::Cutter<CVUniverse, MichelEvent>& michelcuts,
    PlotUtils::Model<CVUniverse, MichelEvent>& model)
{
  assert(!error_bands["cv"].empty() && "\"cv\" error band is empty!  Can't set Model weight.");
  auto& cvUniv = error_bands["cv"].front();

  std::cout << "Starting MC reco loop...\n";
  const int nEntries = chain->GetEntries(); // TODO: July 10 CHANGE BACK TO GETENTRIES
  for (int i=0; i < nEntries; ++i)
  {
    if(i%1000==0) std::cout << i << " / " << nEntries << "\r" << std::flush;
    //std::cout << "Now Printing for Event " << i << std::endl;
    MichelEvent cvEvent;
    cvUniv->SetEntry(i);
    model.SetEntry(*cvUniv, cvEvent);
    const double cvWeight =model.GetWeight(*cvUniv, cvEvent);// TODO: Put this model weight back. model.GetWeight(*cvUniv, cvEvent);

    //=========================================
    // Systematics loop(s)
    //=========================================
    for (auto band : error_bands)
    {
      std::vector<CVUniverse*> error_band_universes = band.second;
      for (auto universe : error_band_universes)
      {
        //if (universe->ShortName() != "cv") continue;
        MichelEvent myevent; // make sure your event is inside the error band loop. 
        //if (universe->ShortName() != "cv") continue;
        // Tell the Event which entry in the TChain it's looking at
        universe->SetEntry(i);
        const double weight2 = model.GetWeight(*universe, myevent); 
        const auto cutResults = michelcuts.isMCSelected(*universe, myevent, cvWeight);
        //const auto cutResults = michelcuts.isDataSelected(*universe, myevent);       
        //if (universe->ShortName() != "cv") continue;
	if (!cutResults.all()) continue;
        if (cutResults.all()){
	    setDistanceMichelSelection(*universe, myevent, 150.);
            setClosestMichel(*universe, myevent,0);
  	    if (!myevent.m_nmichelspass.empty()){
		for(auto& var: vars) {
			//std::cout << "Filling Var for this universe " << i << std::endl;  
		  	//std::cout << " Var Value is " << var->GetRecoValue(*universe) << std::endl;  
        		var->selectedMCReco->FillUniverse(universe, var->GetRecoValue(*universe), weight2); //"Fake data" for closure
            		(*var->m_MChists)[universe->GetInteractionType()].FillUniverse(universe, var->GetRecoValue(*universe), weight2);
            		var->FillCategHistos(*universe,var->GetRecoValue(*universe), weight2);
            		//var->migration->FillUniverse(universe, var->GetRecoValue(*universe), var->GetTrueValue(*universe), weight2);
            		var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValue(*universe), weight2);
            		double reco = var->GetRecoValue(*universe);
            		double truevar = var->GetTrueValue(*universe);
            		double recomtrue = reco - truevar;
            		double bias = recomtrue/truevar;
			var->biasMCReco->FillUniverse(universe, bias, weight2);
			var->recoMinusTrue->FillUniverse(universe, recomtrue, weight2);
        	}
        	for(auto& var: vars2D) {
                	var->mcTotalHist->FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                	(*var->m_MChists)[universe->GetInteractionType()].FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                	var->FillCategHistos(*universe,var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                	//var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValueX(*universe), var->GetTrueValueY(*universe), weight2);
                	double recoX = var->GetRecoValueX(*universe);
                	double truevarX = var->GetTrueValueX(*universe);
                	double recomtrueX = recoX - truevarX;
                	//double recoY = var->GetRecoValueY(*universe);
                	//double truevarY = var->GetTrueValueY(*universe);
                	//double recomtrueY = recoY - truevarY;
                	double biasx = recomtrueX/truevarX;
			var->biasMCReco->FillUniverse(universe, biasx, var->GetRecoValueY(*universe), weight2);
			var->recoMinusTrueRBins->FillUniverse(universe, recomtrueX, var->GetRecoValueY(*universe), weight2);
                	var->recoMinusTrueTBins->FillUniverse(universe, recomtrueX, var->GetTrueValueY(*universe), weight2);
         	}

        	for(auto& study: studies) study->SelectedSignal(*universe, myevent, weight2);

        	const bool isSignal = michelcuts.isSignal(*universe, weight2);

		if(isSignal){
			for(auto& var: vars)
          		{
				var->migration->FillUniverse(universe, var->GetRecoValue(*universe), var->GetTrueValue(*universe), weight2);
                                var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValue(*universe), weight2);
				var->selectedSignalReco->FillUniverse(universe, var->GetRecoValue(*universe), weight2); //Efficiency numerator in reco variables.  Useful for warping studies 
		
			}
			for(auto& var: vars2D)
          		{
            			var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValueX(*universe), var->GetTrueValueY(*universe), weight2);
          		}
		} // end of if Signal()
                else{
          		int bkgd_ID = -1;
          		if (universe->GetCurrent()==2)bkgd_ID=0;
          		else bkgd_ID=1;

          		for(auto& var: vars){

                 		(*var->m_backgroundHists)[bkgd_ID].FillUniverse(universe, var->GetRecoValue(*universe), weight2);
          		}
          		for(auto& var: vars2D) (*var->m_backgroundHists)[bkgd_ID].FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
        	} // End of else statement for if Signal
        } // End of if SelectedMichels Not Empty
        else{
    	
          //myevent.m_nmichels = myevent.m_sidebandpass;
          setDistanceMichelSidebands(*universe, myevent, 150., 500.);
          setClosestMichel(*universe, myevent,1);
	  if (!myevent.m_sidebandpass.empty()){
 	     //const double weight2 = model.GetWeight(*universe, myevent);
             for(auto& var: sidevars) {
                var->selectedMCReco->FillUniverse(universe, var->GetRecoValue(*universe), weight2); //"Fake data" for closure
                (*var->m_MChists)[universe->GetInteractionType()].FillUniverse(universe, var->GetRecoValue(*universe), weight2);
                var->FillCategHistos(*universe,var->GetRecoValue(*universe), weight2);
                var->migration->FillUniverse(universe, var->GetRecoValue(*universe), var->GetTrueValue(*universe), weight2);
                var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValue(*universe), weight2);
                double reco = var->GetRecoValue(*universe);
                double truevar = var->GetTrueValue(*universe);
                double recomtrue = reco - truevar;
                var->recoMinusTrue->FillUniverse(universe, recomtrue, weight2);
             }
             for(auto& var: sidevars2D) {
                var->mcTotalHist->FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                (*var->m_MChists)[universe->GetInteractionType()].FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                var->FillCategHistos(*universe,var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), weight2);
                var->efficiencyNumerator->FillUniverse(universe, var->GetTrueValueX(*universe), var->GetTrueValueY(*universe), weight2);
                double recoX = var->GetRecoValueX(*universe);
                double truevarX = var->GetTrueValueX(*universe);
                double recomtrueX = recoX - truevarX;
                double recoY = var->GetRecoValueY(*universe);
                double truevarY = var->GetTrueValueY(*universe);
                double recomtrueY = recoY - truevarY;
                var->recoMinusTrueRBins->FillUniverse(universe, recomtrueX, var->GetRecoValueY(*universe), weight2);
                var->recoMinusTrueTBins->FillUniverse(universe, recomtrueX, var->GetTrueValueY(*universe), weight2);
             }
             for(auto& study: sideband_studies) study->SelectedSignal(*universe, myevent, weight2);

 	  }
        } // end of else if (!cutResults[0] && evt.sideband == 1) // To fill Sideband Variables
       } // If event passes PreCuts
      } // End band's universe loop
    } // End Band loop
  } //End entries loop
  std::cout << "Finished MC reco loop.\n";
}


void LoopAndFillData( PlotUtils::ChainWrapper* data,
			        std::vector<CVUniverse*> data_band,
				std::vector<Variable*> vars,
                                std::vector<Variable2D*> vars2D,
                                std::vector<Study*> studies,
				std::vector<Variable*> sidevars,
                                std::vector<Variable2D*> sidevars2D,
                                std::vector<Study*> data_sidebands,
				PlotUtils::Cutter<CVUniverse, MichelEvent>& michelcuts)

{
  std::cout << "Starting data loop...\n";
  const int nEntries = data->GetEntries(); // TODO: July 10 CHANGE BACK TO GEtENTRIES
  for (int i=0; i <nEntries; ++i) {
    //std::cout << "Now Printing for Event " << i << std::endl;
    //for (auto universe : data_band) {
      const auto universe = data_band.front();
      universe->SetEntry(i);
      if(i%1000==0) std::cout << i << " / " << nEntries << "\r" << std::flush;
      //std::cout << "Creating Michel Event" << std::endl;
      //if (universe->ShortName() != "cv") continue;
      MichelEvent myevent; 
      //std::cout << "Applying Cuts to Data Event " << std::endl;
      const auto cutResults = michelcuts.isDataSelected(*universe, myevent);
      if (!cutResults.all()) continue;
      if (cutResults.all()){
      	setDistanceMichelSelection(*universe, myevent, 150.);
        setClosestMichel(*universe, myevent,0);
        if (!myevent.m_nmichelspass.empty()){

      		//std::cout << "Filling Data STudies" << std::endl;
      		for(auto& study: studies) study->Selected(*universe, myevent, 1); 

      		for(auto& var: vars)
      		{
 			//std::cout << "Filling Data Var for This universe " << i << std::endl;
          		//std::cout << " Var Value is " << var->GetRecoValue(*universe) << std::endl;
			var->dataHist->FillUniverse(universe, var->GetRecoValue(*universe), 1);
      		}

      		for(auto& var: vars2D)
      		{
        		var->dataHist->FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), 1);
      		}
     	}	 // End of if (michelcuts.isDataSelected(*universe, myevent).all()) -> filling Selection
        else{
          setDistanceMichelSidebands(*universe, myevent, 150., 500.);
          setClosestMichel(*universe, myevent,1);
          if (!myevent.m_sidebandpass.empty()){
        	for(auto& study: data_sidebands) study->Selected(*universe, myevent, 1); 

        	for(auto& var: sidevars)
        	{
          	     var->dataHist->FillUniverse(universe, var->GetRecoValue(*universe), 1);
        	}

        	for(auto& var: sidevars2D)
        	{
          	    var->dataHist->FillUniverse(universe, var->GetRecoValueX(*universe), var->GetRecoValueY(*universe), 1);
        	}
     	} // End of else for Filling Sideband Variables
       } // End of  else if no michelspass
      }// End of PreCuts Pass
    //} // End of Data Band 
   } // End of Entries
  std::cout << "Finished data loop.\n";
}

void LoopAndFillEffDenom( PlotUtils::ChainWrapper* truth,
    				std::map<std::string, std::vector<CVUniverse*> > truth_bands,
    				std::vector<Variable*> vars,
                                std::vector<Variable2D*> vars2D,
 			        std::vector<Study*> studies,
    				PlotUtils::Cutter<CVUniverse, MichelEvent>& michelcuts,
                                PlotUtils::Model<CVUniverse, MichelEvent>& model)
{
  assert(!truth_bands["cv"].empty() && "\"cv\" error band is empty!  Could not set Model entry.");
  auto& cvUniv = truth_bands["cv"].front();

  std::cout << "Starting efficiency denominator loop...\n";
  const int nEntries = truth->GetEntries();
  for (int i=0; i<nEntries; ++i)
  {
    if(i%1000==0) std::cout << i << " / " << nEntries << "\r" << std::flush;
    MichelEvent cvEvent;
    cvUniv->SetEntry(i);
    model.SetEntry(*cvUniv, cvEvent);
    const double cvWeight = model.GetWeight(*cvUniv, cvEvent);

    //=========================================
    // Systematics loop(s)
    //=========================================
    for (auto band : truth_bands)
    {
      std::vector<CVUniverse*> truth_band_universes = band.second;
      for (auto universe : truth_band_universes)
      {
        MichelEvent myevent; //Only used to keep the Model happy

        // Tell the Event which entry in the TChain it's looking at
        universe->SetEntry(i);

        if (!michelcuts.isEfficiencyDenom(*universe, cvWeight)) continue; //Weight is ignored for isEfficiencyDenom() in all but the CV universe 
        const double weight = model.GetWeight(*universe, myevent); //Only calculate the weight for events that will use it

        //Fill efficiency denominator now: 
        for(auto var: vars)
        {
          var->efficiencyDenominator->FillUniverse(universe, var->GetTrueValue(*universe), weight);
        }
        // Fill Studies denominator:
//        for(auto& study: studies) study->SelectedSignal(*universe, cvEvent, weight);

        for(auto var: vars2D)
        {
          var->efficiencyDenominator->FillUniverse(universe, var->GetTrueValueX(*universe), var->GetTrueValueY(*universe), weight);
        }
      }
    }
  }
  std::cout << "Finished efficiency denominator loop.\n";
}

//Returns false if recoTreeName could not be inferred
bool inferRecoTreeNameAndCheckTreeNames(const std::string& mcPlaylistName, const std::string& dataPlaylistName, std::string& recoTreeName)
{
  const std::vector<std::string> knownTreeNames = {"Truth", "Meta", "Header"};
  bool areFilesOK = false;

  std::ifstream playlist(mcPlaylistName);
  std::string firstFile = "";
  playlist >> firstFile;
  auto testFile = TFile::Open(firstFile.c_str());
  if(!testFile)
  {
    std::cerr << "Failed to open the first MC file at " << firstFile << "\n";
    return false;
  }

  //Does the MC playlist have the Truth tree?  This is needed for the efficiency denominator.
  const auto truthTree = testFile->Get("Truth");
  if(truthTree == nullptr || !truthTree->IsA()->InheritsFrom(TClass::GetClass("TTree")))
  {
    std::cerr << "Could not find the \"Truth\" tree in MC file named " << firstFile << "\n";
    return false;
  }

  //Figure out what the reco tree name is
  for(auto key: *testFile->GetListOfKeys())
  {
    if(static_cast<TKey*>(key)->ReadObj()->IsA()->InheritsFrom(TClass::GetClass("TTree"))
       && std::find(knownTreeNames.begin(), knownTreeNames.end(), key->GetName()) == knownTreeNames.end())
    {
      recoTreeName = key->GetName();
      areFilesOK = true;
    }
  }
  delete testFile;
  testFile = nullptr;

  //Make sure the data playlist's first file has the same reco tree
  playlist.open(dataPlaylistName);
  playlist >> firstFile;
  testFile = TFile::Open(firstFile.c_str());
  if(!testFile)
  {
    std::cerr << "Failed to open the first data file at " << firstFile << "\n";
    return false;
  }

  const auto recoTree = testFile->Get(recoTreeName.c_str());
  if(recoTree == nullptr || !recoTree->IsA()->InheritsFrom(TClass::GetClass("TTree")))
  {
    std::cerr << "Could not find the \"" << recoTreeName << "\" tree in data file named " << firstFile << "\n";
    return false;
  }

  return areFilesOK;
}

//==============================================================================
// Main
//==============================================================================
int main(const int argc, const char** argv)
{
  TH1::AddDirectory(false);

  //Validate input.
  //I expect a data playlist file name and an MC playlist file name which is exactly 2 arguments.
  const int nArgsExpected = 2;
  if(argc != nArgsExpected + 1) //argc is the size of argv.  I check for number of arguments + 1 because
                                //argv[0] is always the path to the executable.
  {
    std::cerr << "Expected " << nArgsExpected << " arguments, but got " << argc - 1 << "\n" << USAGE << "\n";
    return badCmdLine;
  }

  //One playlist must contain only MC files, and the other must contain only data files.
  //Only checking the first file in each playlist because opening each file an extra time
  //remotely (e.g. through xrootd) can get expensive.
  //TODO: Look in INSTALL_DIR if files not found?
  const std::string mc_file_list = argv[2],
                    data_file_list = argv[1];
  //Check that necessary TTrees exist in the first file of mc_file_list and data_file_list
  std::string reco_tree_name;
  if(!inferRecoTreeNameAndCheckTreeNames(mc_file_list, data_file_list, reco_tree_name))
  {
    std::cerr << "Failed to find required trees in MC playlist " << mc_file_list << " and/or data playlist " << data_file_list << ".\n" << USAGE << "\n";
    return badInputFile;
  }

  const bool doCCQENuValidation = (reco_tree_name == "CCQENu"); //Enables extra histograms and might influence which systematics I use.

  const bool is_grid = false;
  PlotUtils::MacroUtil options(reco_tree_name, mc_file_list, data_file_list, "minervame1A", true);
  std::cout << options.m_mc->GetChain()->GetName() << std::endl;
  options.m_plist_string = util::GetPlaylist(*options.m_mc, true); //TODO: Put GetPlaylist into PlotUtils::MacroUtil

  // You're required to make some decisions
  PlotUtils::MinervaUniverse::SetNuEConstraint(true);
  PlotUtils::MinervaUniverse::SetPlaylist(options.m_plist_string); //TODO: Infer this from the files somehow?
  PlotUtils::MinervaUniverse::SetAnalysisNuPDG(14);
  PlotUtils::MinervaUniverse::SetNFluxUniverses(100);
  PlotUtils::MinervaUniverse::SetZExpansionFaReweight(false);

  //Now that we've defined what a cross section is, decide which sample and model
  //we're extracting a cross section for.
  PlotUtils::Cutter<CVUniverse, MichelEvent>::reco_t  preCuts;
  PlotUtils::Cutter<CVUniverse, MichelEvent>::reco_t  sidebands;
  PlotUtils::Cutter<CVUniverse, MichelEvent>::truth_t signalDefinition, phaseSpace;

  //const double minZ = 5980, maxZ = 8590.07, apothem = 850.; // All in mm
  const double minZ = 5980, maxZ = 8422, apothem = 850; //All in mm
  //preCuts.emplace_back(new reco::ZRange<CVUniverse, MichelEvent>("Tracker", minZ, maxZ));
  preCuts.emplace_back(new reco::Apothem<CVUniverse, MichelEvent>(apothem));
  preCuts.emplace_back(new reco::ZRange<CVUniverse, MichelEvent>("Tracker", minZ, maxZ));
  preCuts.emplace_back(new reco::MaxMuonAngle<CVUniverse, MichelEvent>(20.));
  preCuts.emplace_back(new reco::HasMINOSMatch<CVUniverse, MichelEvent>());
  preCuts.emplace_back(new reco::NoDeadtime<CVUniverse, MichelEvent>(1, "Deadtime"));
  preCuts.emplace_back(new reco::IsNeutrino<CVUniverse, MichelEvent>());
  //preCuts.emplace_back(new Q3RangeReco<CVUniverse, MichelEvent>(0.0,1.2));
  preCuts.emplace_back(new PTRangeReco<CVUniverse, MichelEvent>(0.0,1.0));

  //preCuts.emplace_back(new hasMichel<CVUniverse, MichelEvent>());
  //preCuts.emplace_back(new BestMichelDistance2D<CVUniverse, MichelEvent>(100.));
  //preCuts.emplace_back(new VtxMatchFirst<CVUniverse, MichelEvent>(200., 102.));
  //preCuts.emplace_back(new NPiCut<CVUniverse, MichelEvent>(1));
  preCuts.emplace_back(new hasMichel<CVUniverse, MichelEvent>());
  //preCuts.emplace_back(new Distance2DSideband<CVUniverse, MichelEvent>(500.));
  
  //preCuts.emplace_back(new BestMichelDistance2D<CVUniverse, MichelEvent>(150.));
  //preCuts.emplace_back(new GetClosestMichel<CVUniverse, MichelEvent>(0));
  //nosidebands.emplace_back(new BestMichelDistance2D<CVUniverse, MichelEvent>(150.));
  //nosidebands.emplace_back(new GetClosestMichel<CVUniverse, MichelEvent>(0));


  TFile* mc_MichelStudies = TFile::Open("July202022_EavailComp_noreweight_q3bin6_MC.root", "RECREATE");
  TFile* data_MichelStudies = TFile::Open("July202022_EavailComp_noreweight_q3bin6_data.root", "RECREATE");
  TFile* mc_SidebandStudies = TFile::Open("July202022_Sideband_noreweight_q3bin6_MC.root", "RECREATE");
  TFile* data_SidebandStudies = TFile::Open("July202022_Sideband_noreweight_q3bin6_data.root", "RECREATE");

  signalDefinition.emplace_back(new truth::IsNeutrino<CVUniverse>());
  signalDefinition.emplace_back(new truth::IsCC<CVUniverse>());
  signalDefinition.emplace_back(new truth::HasPion<CVUniverse>());
  //signalDefinition.emplace_back(new Q3Limit<CVUniverse>(0.0, 1.20));
  
  //signalDefinition.emplace_back(new hasTruePion<CVUniverse>());                                                                                                              
  
  phaseSpace.emplace_back(new truth::ZRange<CVUniverse>("Tracker", minZ, maxZ));
  phaseSpace.emplace_back(new truth::Apothem<CVUniverse>(apothem));
  phaseSpace.emplace_back(new truth::MuonAngle<CVUniverse>(20.));
  phaseSpace.emplace_back(new truth::PZMuMin<CVUniverse>(1500.));
  phaseSpace.emplace_back(new truth::pTRangeLimit<CVUniverse>(0., 1.0));                                                                                                                                                 
  PlotUtils::Cutter<CVUniverse, MichelEvent> mycuts(std::move(preCuts), std::move(sidebands) , std::move(signalDefinition),std::move(phaseSpace));

  std::vector<std::unique_ptr<PlotUtils::Reweighter<CVUniverse, MichelEvent>>> MnvTunev1;
  MnvTunev1.emplace_back(new PlotUtils::FluxAndCVReweighter<CVUniverse, MichelEvent>());
  MnvTunev1.emplace_back(new PlotUtils::GENIEReweighter<CVUniverse, MichelEvent>(true, false));
  MnvTunev1.emplace_back(new PlotUtils::LowRecoil2p2hReweighter<CVUniverse, MichelEvent>());
  MnvTunev1.emplace_back(new PlotUtils::MINOSEfficiencyReweighter<CVUniverse, MichelEvent>());
  MnvTunev1.emplace_back(new PlotUtils::RPAReweighter<CVUniverse, MichelEvent>());
  //TODO: Add my pion reweighter here. - Mehreen S.  Nov 22, 2021
  //MnvTunev1.emplace_back(new PlotUtils::PionReweighter<CVUniverse,MichelEvent>()); 
  PlotUtils::Model<CVUniverse, MichelEvent> model(std::move(MnvTunev1));

  // Make a map of systematic universes
  // Leave out systematics when making validation histograms
  const bool doSystematics = (getenv("MNV101_SKIP_SYST") == nullptr);
  if(!doSystematics){
    std::cout << "Skipping systematics (except 1 flux universe) because environment variable MNV101_SKIP_SYST is set.\n";
    PlotUtils::MinervaUniverse::SetNFluxUniverses(2); //Necessary to get Flux integral later...  Doesn't work with just 1 flux universe though because _that_ triggers "spread errors".
  }

  std::map< std::string, std::vector<CVUniverse*> > error_bands;
  if(doSystematics) error_bands = GetStandardSystematics(options.m_mc);
  else{
    std::map<std::string, std::vector<CVUniverse*> > band_flux = PlotUtils::GetFluxSystematicsMap<CVUniverse>(options.m_mc, CVUniverse::GetNFluxUniverses());
    error_bands.insert(band_flux.begin(), band_flux.end()); //Necessary to get flux integral later...
  }
  error_bands["cv"] = {new CVUniverse(options.m_mc)};
  std::map< std::string, std::vector<CVUniverse*> > truth_bands;
  if(doSystematics) truth_bands = GetStandardSystematics(options.m_truth);
  truth_bands["cv"] = {new CVUniverse(options.m_truth)};

  std::vector<double> dansPTBins = {0, 0.075, 0.10, 0.15, 0.20, 0.30, 0.4, 0.50,0.60 , 0.7, 0.80,0.9, 1.,1.1, 1.2, 1.3, 1.4, 1.5},
                      dansPzBins = {1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 6, 7, 8, 9, 10, 15, 20, 40, 60},
                      robsEmuBins = {0,1,2,3,4,5,7,9,12,15,18,22,36,50,75,80},
                      mehreenQ3Bins = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4},
		      robsRecoilBins;
  
   
  int nq3mbins = mehreenQ3Bins.size() -1; 
  std::vector<double> tpibins = {0, 4., 8., 12., 16., 20., 24., 28., 32., 36., 40., 46., 52.,60., 70., 80., 100., 125.,150., 175., 200., 225., 250., 275., 300., 325., 350., 400., 500., 1000.};   
  std::vector<double> rangebins = {0, 4., 8., 12., 16., 20., 24., 28., 32., 36., 40., 44., 50., 56., 62., 70., 80.,90., 100., 110.,  120., 140., 160., 180., 200., 220., 240., 260., 280., 300., 325., 350., 375., 400., 450., 500., 550., 600., 650., 700., 800., 900., 1000., 1200., 1400., 1800., 2400.};             
  //std::vector<double> tpibins = {0., 4., 8., 12., 16., 20., 24., 28., 32., 36., 40., 46., 52., 60.,  70., 80., 90., 100., 120., 140., 160., 180., 200., 220., 240., 260.,280., 300., 320., 340., 360., 380., 400., 500., 1000.};
  //std::vector<double> rangebins = {0., 4., 8., 12., 16., 20., 24., 28., 32., 36., 40., 46., 52., 60., 70., 80., 90., 100., 120., 140., 160., 180., 200., 220., 260., 280., 300., 320., 340., 360., 380., 400., 425., 450., 475., 500., 600., 700., 800., 1000., 1200., 1400., 1600., 2000., 2400.}; 
  std::vector<double> recoilbins = {0.0, 150., 200., 300., 400., 500., 600., 800., 1000., 1200., 1400., 1600.};
  const double robsRecoilBinWidth = 50; //MeV
  for(int whichBin = 0; whichBin < 30 + 1; ++whichBin) robsRecoilBins.push_back(robsRecoilBinWidth * whichBin);

  std::vector<Variable*> vars = {
    new Variable("pTmu", "p_{T, #mu} [GeV/c]", dansPTBins, &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("q3", "q3 [GeV]", dansPTBins, &CVUniverse::Getq3, &CVUniverse::GetTrueQ3),
    new Variable("q2", "q2 [GeV^2]", dansPTBins, &CVUniverse::GetQ2Reco, &CVUniverse::GetTrueQ2GeV), 
    new Variable("EAvailable", "E_{available}", robsRecoilBins, &CVUniverse::GetEavail, &CVUniverse::GetTrueEAvail),
    new Variable("pzmu", "p_{||, #mu} [GeV/c]", dansPzBins, &CVUniverse::GetMuonPz, &CVUniverse::GetMuonPzTrue),
    new Variable("Emu", "E_{#mu} [GeV]", robsEmuBins, &CVUniverse::GetEmuGeV, &CVUniverse::GetElepTrueGeV),
    new Variable("q3pTdiff","[GeV]", dansPTBins, &CVUniverse::Recoq3pTdiff, &CVUniverse::GetTrueq3pTdiff),
    //new Variable("ERecoilvEavail", "Erecoil", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    //new Variable("ERecoilCorrvEavail", "Erecoilcorr", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0", "Erecoilq0", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    //new Variable("ERecoilCorrvq0", "Erecoilcorrq0", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorr", "Eavailablecorr", robsRecoilBins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail),
    //new Variable("NoPolyLineE", "NoPolyLineE", robsRecoilBins, &CVUniverse::GetNoPolylineE, &CVUniverse::GetTrueQ0),
    //new Variable("PolyLineE", "PolyLineE", robsRecoilBins, &CVUniverse::GetPolylineE, &CVUniverse::GetTrueQ0),
    new Variable("Eavailvq0", "Eavailvq0", robsRecoilBins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueQ0),
    new Variable("NewEAvail", "NewEAvail", robsRecoilBins, &CVUniverse::NewEavail, &CVUniverse::GetTrueEAvail), 
    new Variable("NewRecoilE", "NewRecoilE", robsRecoilBins, &CVUniverse::NewRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("NewEAvailvq0", "NewEavailvq0", robsRecoilBins, &CVUniverse::NewEavail,&CVUniverse::GetTrueQ0)
  };
   
   std::vector<Variable*> varsagain = {
    
    new Variable("pTmu", "pT [GeV/c]", dansPTBins, &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("q3", "q3 [GeV]", mehreenQ3Bins, &CVUniverse::Getq3, &CVUniverse::GetTrueQ3),
    new Variable("q2", "q2 [GeV^2]", dansPTBins, &CVUniverse::GetQ2Reco, &CVUniverse::GetTrueQ2GeV), 
    new Variable("EAvailable", "E_available", robsRecoilBins, &CVUniverse::GetEavail, &CVUniverse::GetTrueEAvail),
    new Variable("pzmu", "p_z [GeV/c]", dansPzBins, &CVUniverse::GetMuonPz, &CVUniverse::GetMuonPzTrue),
    new Variable("Emu", "E_{#mu} [GeV]", robsEmuBins, &CVUniverse::GetEmuGeV, &CVUniverse::GetElepTrueGeV),
    new Variable("pTmubins", "pT [GeV/c]", mehreenQ3Bins,  &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("ERecoilvEavail", "Erecoil", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilCorrvEavail", "Erecoilcorr", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0", "Erecoilq0", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("ERecoilCorrvq0", "Erecoilcorrq0", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorr", "Eavailablecorr", robsRecoilBins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvEavaily", "Erecoily", recoilbins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilCorrvEavaily", "Erecoilcorry", recoilbins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0y", "Erecoilq0y", recoilbins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("ERecoilCorrvq0y", "Erecoilcorrq0y", recoilbins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorry", "Eavailablecorry", recoilbins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail),
    new Variable("EavailCorryq0", "Eavailablecorryq0", recoilbins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueQ0), 
    new Variable("NewEAvail", "NewEAvail", robsRecoilBins, &CVUniverse::NewEavail, &CVUniverse::GetTrueEAvail),  
    new Variable("NewRecoilE", "NewRecoilE", robsRecoilBins, &CVUniverse::NewRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("NewEAvailvq0", "NewEavailvq0", robsRecoilBins, &CVUniverse::NewEavail,&CVUniverse::GetTrueQ0),
    new Variable("NewEAvaily", "NewEAvaily", recoilbins, &CVUniverse::NewEavail, &CVUniverse::GetTrueEAvail),
    new Variable("NewRecoilEy", "NewRecoilEy", recoilbins, &CVUniverse::NewRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("NewEAvailvq0y", "NewEavailvq0y", recoilbins, &CVUniverse::NewEavail,&CVUniverse::GetTrueQ0)
  };

  std::vector<Variable2D*> vars2D;
  vars2D.push_back(new Variable2D(*varsagain[6], *varsagain[1])); // ptmu bins vs pt
  vars2D.push_back(new Variable2D(*varsagain[6], *varsagain[3]));// ptmubins vs Eavail For some reason this is not printing correctly
  //vars2D.push_back(new Variable2D(*varsagain[1], *varsagain[3]));// ptmu vs Eavail
  vars2D.push_back(new Variable2D(*varsagain[6], *varsagain[2])); // ptmu bins vs q3
  vars2D.push_back(new Variable2D(*varsagain[6], *varsagain[4])); // ptmu bins vs pz
  //vars2D.push_back(new Variable2D(*varsagain[1], *varsagain[2])); // ptmu bins vs q2
  //vars2D.push_back(new Variable2D(*varsagain[1], *varsagain[4])); 
  //vars2D.push_back(new Variable2D(*varsagain[1], *varsagain[0]));  
  //vars2D.push_back(new Variable2D(*varsagain[3], *varsagain[0]));
  //vars2D.push_back(new Variable2D(*varsagain[3], *varsagain[3]));
  vars2D.push_back(new Variable2D(*varsagain[7], *varsagain[12]));
  //vars2D.push_back(new Variable2D(*varsagain[8], *varsagain[13]));
  vars2D.push_back(new Variable2D(*varsagain[9], *varsagain[14])); 
  //vars2D.push_back(new Variable2D(*varsagain[10], *varsagain[15]));
  vars2D.push_back(new Variable2D(*varsagain[11], *varsagain[16]));
  vars2D.push_back(new Variable2D(*varsagain[3], *varsagain[17])); // Eavail vs true q0 in recoilbins
  vars2D.push_back(new Variable2D(*varsagain[18], *varsagain[21])); //New Eavail vs true Eavail in recoil bins y;
  vars2D.push_back(new Variable2D(*varsagain[19], *varsagain[22])); // New RecoilE vs true q0 in recoil bins y;
  vars2D.push_back(new Variable2D(*varsagain[20], *varsagain[23])); // New Eavail vs true q0 in recoil bins y;
   


  std::vector<Variable*> sidevars = {
    new Variable("pTmu", "p_{T, #mu} [GeV/c]", dansPTBins, &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("q3", "q3 [GeV]", dansPTBins, &CVUniverse::Getq3, &CVUniverse::GetTrueQ3),
    new Variable("q2", "q2 [GeV^2]", dansPTBins, &CVUniverse::GetQ2Reco, &CVUniverse::GetTrueQ2GeV),
    new Variable("EAvailable", "E_{available}", robsRecoilBins, &CVUniverse::GetEavail, &CVUniverse::GetTrueEAvail),
    new Variable("pzmu", "p_{||, #mu} [GeV/c]", dansPzBins, &CVUniverse::GetMuonPz, &CVUniverse::GetMuonPzTrue),
    new Variable("Emu", "E_{#mu} [GeV]", robsEmuBins, &CVUniverse::GetEmuGeV, &CVUniverse::GetElepTrueGeV),
    new Variable("q3pTdiff","[GeV]", dansPTBins, &CVUniverse::Recoq3pTdiff, &CVUniverse::GetTrueq3pTdiff),
    new Variable("ERecoilvEavail", "Erecoil", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilCorrvEavail", "Erecoilcorr", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0", "Erecoilq0", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("ERecoilCorrvq0", "Erecoilcorrq0", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorr", "Eavailablecorr", robsRecoilBins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail)
   };

   std::vector<Variable*> sidevarsagain = {
    
    new Variable("pTmu", "pT [GeV/c]", dansPTBins, &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("q3", "q3 [GeV]", mehreenQ3Bins, &CVUniverse::Getq3, &CVUniverse::GetTrueQ3),
    new Variable("q2", "q2 [GeV^2]", dansPTBins, &CVUniverse::GetQ2Reco, &CVUniverse::GetTrueQ2GeV), 
    new Variable("EAvailable", "E_available", robsRecoilBins, &CVUniverse::GetEavail, &CVUniverse::GetTrueEAvail),
    new Variable("pzmu", "p_z [GeV/c]", dansPzBins, &CVUniverse::GetMuonPz, &CVUniverse::GetMuonPzTrue),
    new Variable("Emu", "E_{#mu} [GeV]", robsEmuBins, &CVUniverse::GetEmuGeV, &CVUniverse::GetElepTrueGeV),
    new Variable("pTmubins", "pT [GeV/c]", mehreenQ3Bins,  &CVUniverse::GetMuonPT, &CVUniverse::GetMuonPTTrue),
    new Variable("ERecoilvEavail", "Erecoil", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilCorrvEavail", "Erecoilcorr", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0", "Erecoilq0", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("ERecoilCorrvq0", "Erecoilcorrq0", robsRecoilBins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorr", "Eavailablecorr", robsRecoilBins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvEavaily", "Erecoily", recoilbins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilCorrvEavaily", "Erecoilcorry", recoilbins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueEAvail),
    new Variable("ERecoilvq0y", "Erecoilq0y", recoilbins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueQ0),
    new Variable("ERecoilCorrvq0y", "Erecoilcorrq0y", recoilbins, &CVUniverse::GetRecoilECorr, &CVUniverse::GetTrueQ0),
    new Variable("EavailCorry", "Eavailablecorry", recoilbins, &CVUniverse::GetEavailCorr, &CVUniverse::GetTrueEAvail)
  };

  std::vector<Variable2D*> sidevars2D;
  sidevars2D.push_back(new Variable2D(*sidevarsagain[6], *sidevarsagain[1]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[6], *sidevarsagain[3]));//For some reason this is not printing correctly
  sidevars2D.push_back(new Variable2D(*sidevarsagain[1], *sidevarsagain[3]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[6], *sidevarsagain[2]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[6], *sidevarsagain[4]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[1], *sidevarsagain[2]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[1], *sidevarsagain[4]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[1], *sidevarsagain[0]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[3], *sidevarsagain[0]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[3], *sidevarsagain[3]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[7], *sidevarsagain[12]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[8], *sidevarsagain[13]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[9], *sidevarsagain[14]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[10], *sidevarsagain[15]));
  sidevars2D.push_back(new Variable2D(*sidevarsagain[11], *sidevarsagain[16]));



 //vars2D.push_back(new Variable2D(vars[3],vars[0]));
  if(doCCQENuValidation)
  {
    std::cerr << "Detected that tree name is CCQENu.  Making validation histograms.\n";
    vars.push_back(new Variable("pzmu", "p_{||, #mu} [GeV/c]", dansPzBins, &CVUniverse::GetMuonPz, &CVUniverse::GetMuonPzTrue));
    vars.push_back(new Variable("Emu", "E_{#mu} [GeV]", robsEmuBins, &CVUniverse::GetEmuGeV, &CVUniverse::GetElepTrueGeV));
    vars.push_back(new Variable("Erecoil", "E_{recoil}", robsRecoilBins, &CVUniverse::GetRecoilE, &CVUniverse::GetTrueEAvail)); //TODO: q0 is not the same as recoil energy without a spline correction
    
    vars2D.push_back(new Variable2D(*vars[1], *vars[0]));
  }
  
  //vars2D.push_back(new Variable2D("q3_vs_pT", "q3_vs_pT;q3;pT", &CVUniverse::GetMuonPT, &CVUniverse::Getq3, dansPTBins,dansPTBins));
//This is where your list of Studies go for PerMichel variables -> Accessed through MichelEvent

  std::vector<Study*> studies;
  std::vector<Study*> sideband_studies; 
   
  std::function<double(const CVUniverse&, const MichelEvent&)> getq3 = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   double q3 = evt.q3_reco;
				   //std::cout << "Printing q3 " << q3 << std::endl;
                                   return q3;
                                 }; 
  std::function<double(const CVUniverse&, const MichelEvent&)> getpT = [](const CVUniverse& univ, const MichelEvent& evt)
                                 { 
                                   double pT = evt.pT_reco;
				   //std::cout << "Printing pT " << pT  << std::endl;
                                   return pT;
                                 }; 
  std::function<double(const CVUniverse&, const MichelEvent&)> getEavail = [](const CVUniverse& univ, const MichelEvent& evt)
                                 { 
                                   double eavail = evt.eavail_reco;
				   //std::cout << "Printing eavail " << eavail << std::endl;
                                   return eavail;
                                 }; 
 
  std::function<double(const CVUniverse&, const MichelEvent&, const int)> delta_t = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   int evttype = evt.eventtype;
                                   double micheltime = evt.m_nmichels[whichMichel].time;
                                   double vtxtime = univ.GetVertex().t();
                                   double deltat = (micheltime - vtxtime/1000.); //hopefully this is in microseconds (mus)
                                   //if (evttype == 1) return deltat;
                                   //else return -9999.;
                                   return deltat;
                                 };

   std::function<double(const CVUniverse&, const MichelEvent&, const int)> permichel_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   //if (evt.eventtype == 1) return micheldist;
                                   //else return -9999.;
                                   return micheldist;
                                 };
   std::function<double(const CVUniverse&, const MichelEvent&, const int)> pertruepimichel_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
				   if (evt.m_nmichels[whichMichel].true_parentpdg == 211) return micheldist;
 				   else return -9999.;
                                 };
   std::function<double(const CVUniverse&, const MichelEvent&, const int)> permichel_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
				   if (evt.m_nmichels[whichMichel].true_parentpdg == 211) return evt.m_nmichels[whichMichel].pionKE;
				   else return 9999.;

				 };

  std::function<double(const CVUniverse&, const MichelEvent&, const int)> overlay_vtx_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
				   double KE = evt.m_nmichels[whichMichel].pionKE;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   if (overlayfrac > .5 && (matchtype == 1 || matchtype == 2 )) return KE;
                                   else return -9999.;
                                 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> overlay_clus_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   double KE = evt.m_nmichels[whichMichel].pionKE;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   if (overlayfrac > .5 && (matchtype == 3 || matchtype == 4 )) return KE;
                                   else return -9999.;
                                 };

 std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_goodvtx_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   double KE = evt.m_nmichels[whichMichel].pionKE;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 1 || matchtype == 2)) return KE;
                                   else return -9999.;
                                 };
 std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_goodclus_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   double KE = evt.m_nmichels[whichMichel].pionKE;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 3 || matchtype == 4)) return KE;
                                   else return -9999.;
                                 };
  std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_badvtx_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   double KE = evt.m_nmichels[whichMichel].pionKE;
				   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 1 || matchtype == 2))
                                   {
                                        //univ.PrintArachneLink();
                                        //std::cout << "Printing Michel Time for bad VTX match type "  << evt.m_nmichels[whichMichel].time << std::endl;
                                        return KE;
                                   }
                                   else return -9999.;
                                 };

 std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_badclus_tpi = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                   double KE = evt.m_nmichels[whichMichel].pionKE;
				   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 3 || matchtype == 4))
                                   {
                                         //univ.PrintArachneLink();
                                         //std::cout << "Printing Michel Time for bad CLUS match type "  << evt.m_nmichels[whichMichel].time << std::endl;
                                         return KE;
                                   }
                                   else return -9999.;
                                 };


                                  
   std::function<double(const CVUniverse&, const MichelEvent&, const int)> overlay_vtx_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;

				   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
	                           int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
	 			   if (overlayfrac > .5 && (matchtype == 1 || matchtype == 2 )) return micheldist;
 				   else return -9999.;
				 };

   std::function<double(const CVUniverse&, const MichelEvent&, const int)> overlay_clus_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;

                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   if (overlayfrac > .5 && (matchtype == 3 || matchtype == 4 )) return micheldist;
                                   else return -9999.;
                                 };

    std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_goodvtx_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
			           int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;

                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
		                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 1 || matchtype == 2)) return micheldist;
                                   else return -9999.;
                                 }; 

   std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_goodclus_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
				   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 3 || matchtype == 4)) return micheldist;
                                   else return -9999.;
                                 }; 

  std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_badvtx_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
				   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 1 || matchtype == 2))
			           {
				  //      univ.PrintArachneLink();
				//	std::cout << "Printing Michel Time for bad VTX match type "  << evt.m_nmichels[whichMichel].time << std::endl;
					return micheldist;
                                   }
				   else return -9999.;
                                 }; 
 std::function<double(const CVUniverse&, const MichelEvent&, const int)> truemichel_badclus_range = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
				   int overlayfrac = evt.m_nmichels[whichMichel].overlay_fraction;
                                   int matchtype = evt.m_nmichels[whichMichel].BestMatch;
                                   int trueEnd = evt.m_nmichels[whichMichel].trueEndpoint;
                                   int recoEnd = evt.m_nmichels[whichMichel].recoEndpoint;
                                   if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 3 || matchtype == 4))
			           {
				//	 univ.PrintArachneLink();
                                  //       std::cout << "Printing Michel Time for bad CLUS match type "  << evt.m_nmichels[whichMichel].time << std::endl;
					 return micheldist;
                                   }
				   else return -9999.; 
                                 };


std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_XZ = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 { 
				   double twoDdist = evt.m_nmichels[whichMichel].best_XZ;
            			   return twoDdist;
		   		 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_UZ = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].best_UZ;
                                   return twoDdist;
                                 };


std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_VZ = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].best_VZ;
                                   return twoDdist;
                                 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_XZ_upvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_vertex_XZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
				   if (trueend == 1) return twoDdist;
  				   else return 9999.;
                                 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_UZ_upvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_vertex_UZ;
				   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 1) return twoDdist;
                                   else return 9999.;
                                 };


std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_VZ_upvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_vertex_VZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 1) return twoDdist;
                                   else return 9999.;
				 };


std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_XZ_upclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_clus_XZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 1) return twoDdist;
                                   else return 9999.;
				 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_UZ_upclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_clus_UZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 1) return twoDdist;
                                   else return 9999.; 
				};

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_VZ_upclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].up_to_clus_VZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 1) return twoDdist;
                                   else return 9999.;
				 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_XZ_downclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_clus_XZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.; 
                                 };
std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_UZ_downclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_clus_UZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.; 
                                 };
std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_VZ_downclus = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_clus_VZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.; 
                                 };
std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_XZ_downvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_vertex_XZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.; 
                                 };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_UZ_downvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_vertex_UZ;
				   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.;
                                 };



std::function<double(const CVUniverse&, const MichelEvent&, const int)> michel_VZ_downvtx = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                 {
                                   double twoDdist = evt.m_nmichels[whichMichel].down_to_vertex_VZ;
                                   int trueend = evt.m_nmichels[whichMichel].trueEndpoint;
                                   if (trueend == 2) return twoDdist;
                                   else return 9999.;  
                                 };


std::function<double(const CVUniverse&, const MichelEvent&, const int)> pion_angle = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
				{
    				  double angle = evt.m_nmichels[whichMichel].best_angle;
 				  return cos(angle);
				};

std::function<double(const CVUniverse&, const MichelEvent&, const int)> pion_angle_range1 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].best_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
   				  if (micheldist <= 150.) return cos(angle);
				  else return 9999.;
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> pion_angle_range2 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].best_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist >150. && micheldist <= 250.) return cos(angle);
                                  else return 9999.;
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> pion_angle_range3 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].best_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist > 250. && micheldist <=500.) return cos(angle);
                                  else return 9999.;
                                };
std::function<double(const CVUniverse&, const MichelEvent&, const int)> pion_angle_range4 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].best_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist > 500.) return cos(angle);
                                  else return 9999.;
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> true_angle = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].true_angle;
                                  return cos(angle);
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> true_angle_range1 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].true_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist <= 150.) return cos(angle);
                                  else return 9999.;
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> true_angle_range2 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].true_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist > 150 && micheldist <= 250.) return cos(angle);
                                  else return 9999.;
                                };
std::function<double(const CVUniverse&, const MichelEvent&, const int)> true_angle_range3 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].true_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist > 250 && micheldist <= 500.) return cos(angle);
                                  else return 9999.;
                                };

std::function<double(const CVUniverse&, const MichelEvent&, const int)> true_angle_range4 = [](const CVUniverse& univ, const MichelEvent& evt, const int whichMichel)
                                {
                                  double angle = evt.m_nmichels[whichMichel].true_angle;
                                  double  micheldist = evt.m_nmichels[whichMichel].Best3Ddist;
                                  if (micheldist > 500) return cos(angle);
                                  else return 9999.;
                                };

  //std::vector<double> rangebins = {0., 15., 30., 45., 60., 75., 100., 130., 160., 190., 220., 260., 300., 350., 400., 450., 500., 550., 600., 650., 700., 800., 900., 1000., 1400., 1800., 2400.};
   int nbinsrange = rangebins.size()-1;
   int nbinstpi = tpibins.size()-1;

  // studies.push_back(new PerMichelVarByGENIELabel(true_angle, "true_angle", "cos(#theta)", 21, -1.0, 1.0, error_bands));
  // studies.push_back(new PerMichelVarByGENIELabel(true_angle_range1, "true_angle_range1", "cos(#theta)", 21, -1.0, 1., error_bands));
  // studies.push_back(new PerMichelVarByGENIELabel(true_angle_range2, "true_angle_range2", "cos(#theta)", 21, -1.0, 1., error_bands));
  // studies.push_back(new PerMichelVarByGENIELabel(true_angle_range3, "true_angle_range3", "cos(#theta)", 21, -1.0, 1., error_bands));
  // studies.push_back(new PerMichelVarByGENIELabel(true_angle_range4, "true_angle_range4", "cos(#theta)", 21, -1.0, 1., error_bands));
   //studies.push_back(new PerMichelVarByGENIELabel(pion_angle, "pion_angle", "cos(#theta)", 21, -1.0, 1., error_bands));
   //studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range1, "pion_angle_range1", "cos(#theta)", 21, -1.0, 1., error_bands));
   //studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range2, "pion_angle_range2", "cos(#theta)", 21, -1.0, 1., error_bands));
   //studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range3, "pion_angle_range3", "cos(#theta)", 21, -1.0, 1., error_bands));
   //studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range4, "pion_angle_range4", "cos(#theta)", 21, -1.0, 1., error_bands));

   studies.push_back(new PerMichelVarVec(michel_XZ, "best_XZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_UZ, "best_UZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_VZ, "best_VZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_XZ_upvtx, "upvtx_XZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_XZ_upclus, "upclus_XZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_XZ_downclus, "downclus_XZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_XZ_downvtx, "downvtx_XZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_UZ_upvtx, "upvtx_UZ", "mm", nbinsrange,rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_UZ_upclus, "upclus_UZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_UZ_downclus, "downclus_UZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_UZ_downvtx, "downvtx_UZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_VZ_upvtx, "upvtx_VZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_VZ_upclus, "upclus_VZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_VZ_downclus, "downclus_VZ", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(michel_VZ_downvtx, "downvtx_VZ", "mm", nbinsrange, rangebins, error_bands));

  // studies.push_back(new PerMichelVarByGENIELabel(delta_t, "michelmuon_deltat", "#mus", 30, 0.0, 9.0, error_bands));
   studies.push_back(new PerMichelVarVec(overlay_vtx_range, "overlay_vtx_range",  "mm", nbinsrange, rangebins, error_bands));    
   studies.push_back(new PerMichelVarVec(overlay_clus_range, "overlay_clus_range",  "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(truemichel_goodvtx_range, "truemichel_goodvtx_range",  "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(truemichel_goodclus_range, "truemichel_goodclus_range",  "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(truemichel_badvtx_range, "truemichel_badvtx_range",  "mm",  nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(truemichel_badclus_range, "truemichel_badclus_range",  "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(permichel_tpi, "Per_Michel_PrimaryParentKE", "MeV", nbinstpi, tpibins, error_bands));
   studies.push_back(new PerMichelVarVec(pertruepimichel_range, "permichel_pirange_truepi", "mm", nbinstpi, tpibins, error_bands)); 

   //VarConfig deltat_config{"deltat", "#mus", 30, 0., 9.};
   VarConfig pirange_config{"pirange", "mm", 100, 0.0, 2000.0};
   VarConfig tpi_config{"KE", "MeV", 100, 0.0, 1000.};
   VarConfig2D rangeconfig{"Pion_Range", "mm", nbinsrange, rangebins};
   VarConfig2D tpiconfig{"Pion_KE", "MeV", nbinstpi, tpibins};
   int nbinq3 = dansPTBins.size()-1;
   eVarConfig2D q3config{"q3_reco", "GeV", nq3mbins, mehreenQ3Bins};
   eVarConfig2D pTconfig{"pT_reco", "GeV", nq3mbins, mehreenQ3Bins};
   eVarConfig2D erangeconfig{"ePirange", "mm", nbinsrange, rangebins};
   eVarConfig2D etpiconfig{"ePion_KE", "MeV", nbinstpi, tpibins};
   std::vector<double> anglebins = {-1.2,-1, -.80, -.70, -.60, -.50, -.40, -.30, -.20, -.10, 0., .10, .20, .30, .40,.50,.60,.70,.80,.90,1.,1.2};
   int nbinsangle = anglebins.size() -1;
   studies.push_back(new PerMichelVarVecFSPart(pion_angle, "pion_angle", "cos(#theta)", nbinsangle,anglebins, error_bands));
   studies.push_back(new PerMichelVarVecFSPart(pion_angle_range1, "pion_angle_range1", "cos(#theta)", nbinsangle,anglebins, error_bands));
   studies.push_back(new PerMichelVarVecFSPart(pion_angle_range2, "pion_angle_range2", "cos(#theta)", nbinsangle,anglebins, error_bands));
   studies.push_back(new PerMichelVarVecFSPart(pion_angle_range3, "pion_angle_range3", "cos(#theta)", nbinsangle,anglebins, error_bands));
   studies.push_back(new PerMichelVarVecFSPart(pion_angle_range4, "pion_angle_range4", "cos(#theta)",nbinsangle,anglebins , error_bands));

   studies.push_back(new PerMichelVarVec(permichel_range,"permichel_pirange", "mm",nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVec(permichel_tpi,"permichel_tpi", "MeV", nbinstpi, tpibins, error_bands)); 
   studies.push_back(new PerMichelVarVecFSPart(permichel_range, "permichel_pirange_npi", "mm", nbinsrange, rangebins, error_bands));
   studies.push_back(new PerMichelVarVecFSPart(permichel_tpi,"permichel_tpi_npi", "MeV", nbinstpi, tpibins, error_bands));

   sideband_studies.push_back(new PerMichelVarVecFSPart(permichel_range, "permichel_pirange_npi", "mm", nbinsrange, rangebins, error_bands));
   sideband_studies.push_back(new PerMichelVarVecFSPart(permichel_tpi,"permichel_tpi_npi", "MeV", nbinstpi, tpibins, error_bands));
   sideband_studies.push_back(new PerMichel2DVarbin(permichel_tpi, permichel_range, tpiconfig, rangeconfig, error_bands));
   sideband_studies.push_back(new PerMichel2DVar(permichel_tpi, permichel_range, tpi_config, pirange_config, error_bands));

   studies.push_back(new PerMichel2DVarbin(permichel_tpi, permichel_range, tpiconfig, rangeconfig, error_bands));
   studies.push_back(new PerMichel2DVar(permichel_tpi, permichel_range, tpi_config, pirange_config, error_bands));
   
   std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                   double dist = evt.m_nmichels[bestidx].Best3Ddist;
                                   return dist;
                                   }
                                 };

   std::function<double(const CVUniverse&, const MichelEvent&)> best_tpi = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
				   if (bestidx < 0) return -9999.;
                                   else{
                                   double tpi = evt.m_nmichels[bestidx].pionKE;
                                   return tpi;
                                   }
                                 };
   std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_overlay_vtx = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                   	int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                   	int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                   	int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                   	int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                   	if (overlayfrac > .5 && (matchtype == 1 || matchtype == 2)) return micheldist;
                                   	else return -9999.;
				   }
                                 };

  std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_overlay_clus = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                        int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                        int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                        int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                        int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                        if (overlayfrac > .5 && (matchtype == 3 || matchtype == 4)) return micheldist;
                                        else return -9999.;
                                   }
                                 };

std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_truegood_vtx = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                        int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                        int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                        int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                        int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                        if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 1 || matchtype == 2)) return micheldist;
                                        else return -9999.;
                                   }
                                 };

std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_truebad_vtx = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                        int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                        int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                        int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                        int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                        if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 1 || matchtype == 2)) return micheldist;
                                        else return -9999.;
                                   }
                                 };

std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_truegood_clus = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                        int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                        int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                        int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                        int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                        if (overlayfrac < .5 && trueEnd == recoEnd && (matchtype == 3 || matchtype == 4)) return micheldist;
                                        else return -9999.;
                                   }
                                 };

std::function<double(const CVUniverse&, const MichelEvent&)> best_pionrange_truebad_clus = [](const CVUniverse& univ, const MichelEvent& evt)
                                 {
                                   int bestidx = evt.m_idx;
                                   if (bestidx < 0) return -9999.;
                                   else{
                                        double micheldist = evt.m_nmichels[bestidx].Best3Ddist;
                                        int overlayfrac = evt.m_nmichels[bestidx].overlay_fraction;
                                        int matchtype = evt.m_nmichels[bestidx].BestMatch;
                                        int trueEnd = evt.m_nmichels[bestidx].trueEndpoint;
                                        int recoEnd = evt.m_nmichels[bestidx].recoEndpoint;
                                        if (overlayfrac < .5 && trueEnd != recoEnd && (matchtype == 3 || matchtype == 4)) return micheldist;
                                        else return -9999.;
                                   }
                                 };


std::function<double(const CVUniverse&, const MichelEvent&)> lowesttpi = [](const CVUniverse& univ, const MichelEvent& evt)
{
				   double lowtpi = evt.lowTpi;
			  	   return lowtpi;

};
   studies.push_back(new PerEventVarBin(lowesttpi, "LowestKE_pion", "MeV", nbinstpi ,tpibins, error_bands));

   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange, "best_pionrange", "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_overlay_clus, "best_pionrange_overlay_clus",  "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_overlay_vtx , "best_pionrange_overlay_vtx ",  "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_truegood_vtx, "best_pionrange_truegood_vtx",  "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_truebad_vtx, "best_pionrange_truebad_vtx",  "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_truegood_clus, "best_pionrange_truegood_clus",  "mm", 100, 0.0, 2000.0, error_bands));
   studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange_truebad_clus, "best_pionrange_truebad_clus",  "mm", 100, 0.0, 2000.0, error_bands));
   
   studies.push_back(new PerMichelEvent2DVarbin(best_tpi, best_pionrange, etpiconfig, erangeconfig, error_bands));
   studies.push_back(new PerMichelEvent2DVarbin(best_tpi, getq3, etpiconfig, q3config, error_bands));
   studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getq3, erangeconfig, q3config, error_bands));
   studies.push_back(new PerMichelEvent2DVarbin(best_tpi, getpT, etpiconfig, pTconfig, error_bands));
   studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getpT, erangeconfig, pTconfig, error_bands));

   sideband_studies.push_back(new PerMichelEvent2DVarbin(best_tpi, best_pionrange, etpiconfig, erangeconfig, error_bands));
   sideband_studies.push_back(new PerMichelEvent2DVarbin(best_tpi, getq3, etpiconfig, q3config, error_bands));
   sideband_studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getq3, erangeconfig, q3config, error_bands));
   sideband_studies.push_back(new PerMichelEvent2DVarbin(best_tpi, getpT, etpiconfig, pTconfig, error_bands));
   sideband_studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getpT, erangeconfig, pTconfig, error_bands)); 



  //studies.push_back(new PerMichel2DVar(delta_t, permichel_range, deltat_config, pirange_config, error_bands));  
   //   studies.push_back(new PerMichel2DVar(permichel_tpi, permichel_range, tpi_config, pirange_config, error_bands));

// Set Up Data Universe

  CVUniverse* data_universe = new CVUniverse(options.m_data);
  std::vector<CVUniverse*> data_band = {data_universe};
  std::map<std::string, std::vector<CVUniverse*> > data_error_bands;
  data_error_bands["cv"] = data_band;
  
  std::vector<Study*> data_studies;
  std::vector<Study*> data_sidebands;
  data_studies.push_back(new PerMichelEventVarByGENIELabel(best_pionrange, "best_pionrange", "mm", 100, 0.0, 2000.0, data_error_bands));
 // data_studies.push_back(new PerMichelVarByGENIELabel(delta_t, "michelmuon_deltat", "#mus", 30, 0.0, 9.0, data_error_bands));
 // data_studies.push_back(new PerMichelVarByGENIELabel(pion_angle, "pion_angle", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 // data_studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range1, "pion_angle_range1", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 // data_studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range2, "pion_angle_range2", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 /// data_studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range3, "pion_angle_range3", "cos(#theta)", 21, -1.0, 1., data_error_bands));
  //data_studies.push_back(new PerMichelVarByGENIELabel(pion_angle_range4, "pion_angle_range4", "cos(#theta)", 21, -1.0, 1., data_error_bands)); 
  data_studies.push_back(new PerMichelVarVec(permichel_range,"permichel_pirange", "mm",nbinsrange, rangebins, data_error_bands));
  data_studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getq3, erangeconfig, q3config, data_error_bands));
  data_studies.push_back(new PerMichelEvent2DVarbin(best_pionrange, getpT, erangeconfig, pTconfig, data_error_bands));
 

  data_sidebands.push_back(new PerMichelEventVarByGENIELabel(best_pionrange, "best_pionrange", "mm", 100, 0.0, 2000.0, data_error_bands));
 // data_sidebands.push_back(new PerMichelVarByGENIELabel(delta_t, "michelmuon_deltat", "#mus", 30, 0.0, 9.0, data_error_bands));
 // data_sidebands.push_back(new PerMichelVarByGENIELabel(pion_angle, "pion_angle", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 // data_sidebands.push_back(new PerMichelVarByGENIELabel(pion_angle_range1, "pion_angle_range1", "cos(#theta)", 21, -1.0, 1., data_error_bands));
//  data_sidebands.push_back(new PerMichelVarByGENIELabel(pion_angle_range2, "pion_angle_range2", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 // data_sidebands.push_back(new PerMichelVarByGENIELabel(pion_angle_range3, "pion_angle_range3", "cos(#theta)", 21, -1.0, 1., data_error_bands));
 // data_sidebands.push_back(new PerMichelVarByGENIELabel(pion_angle_range4, "pion_angle_range4", "cos(#theta)", 21, -1.0, 1., data_error_bands));
  data_sidebands.push_back(new PerMichelVarVec(permichel_range,"permichel_pirange", "mm",nbinsrange, rangebins, data_error_bands));
  data_sidebands.push_back(new PerMichelEvent2DVarbin(best_pionrange, getq3, erangeconfig, q3config, data_error_bands));
  data_sidebands.push_back(new PerMichelEvent2DVarbin(best_pionrange, getpT, erangeconfig, pTconfig, data_error_bands)); 
   

  for(auto& var: vars) var->InitializeMCHists(error_bands, truth_bands);
  for(auto& var: vars) var->InitializeDATAHists(data_band);
  for(auto& var: sidevars) var->InitializeDATAHists(data_band);
  for(auto& var: sidevars) var->InitializeMCHists(error_bands, truth_bands);
  for(auto& var: vars2D) var->InitializeMCHists(error_bands, truth_bands);
  for(auto& var: vars2D) var->InitializeDATAHists(data_band);
  for(auto& var: sidevars2D) var->InitializeMCHists(error_bands, truth_bands);
  for(auto& var: sidevars2D) var->InitializeDATAHists(data_band);
   

  // Loop entries and fill
  try
  {
    CVUniverse::SetTruth(false);
    LoopAndFillEventSelection(options.m_mc, error_bands, vars, vars2D, studies, sidevars, sidevars2D, sideband_studies, mycuts, model);
    std::cout << "MC cut summary:\n" << mycuts << "\n";
    //mycuts.resetStats();
    CVUniverse::SetTruth(true);
    //LoopAndFillEffDenom(options.m_truth, truth_bands, vars, vars2D,studies, mycuts, model);
    //options.PrintMacroConfiguration(argv[0]);
    //std::cout << "Sideband Cut Summary:\n" << mysides << "\n";
    mycuts.resetStats();
    CVUniverse::SetTruth(false);
    LoopAndFillData(options.m_data, data_band, vars, vars2D, data_studies, sidevars, sidevars2D, data_sidebands, mycuts);
    std::cout << "Data cut summary:\n" << mycuts << "\n";
    //mycuts.resetStats();
    
    std::cout  << "Opening the MCOUTPUT for EVENT hists" << std::endl;
    //Write MC results
    TFile* mcOutDir = TFile::Open(MC_OUT_FILE_NAME, "RECREATE");
    TFile* mcSideDir = TFile::Open(MC_SIDE_FILE_NAME, "RECREATE");
    if(!mcOutDir)
    {
      std::cerr << "Failed to open a file named " << MC_OUT_FILE_NAME << " in the current directory for writing histograms.\n";
      return badOutputFile;
    }
    
    if(!mcSideDir)
    {
      std::cerr << "Failed to open a file named " << MC_SIDE_FILE_NAME << " in the current directory for writing histograms.\n";
      return badOutputFile;
    }
    
    std::cout << "Saving Histos to MC Files" << std::endl;
    //TFile* mc_MichelStudies = TFile::Open("AllMichel_hasFittedMichel_500mm.root", "RECREATE");
    //"ALL2DDistprinted_OnlyPionMichels_tpimorethan80meV_forceendpointmatch_2Ddistcut_mc.root", "RECREATE");
    for(auto& study: studies) study->SaveOrDraw(*mc_MichelStudies);
    std::cout << "WRiting VARS to event level file" << std::endl;
    for(auto& var: vars) var->WriteMC(*mcOutDir);
    std::cout << "WRiting 2D VARS to event level file " << std::endl;
    for(auto& var: vars2D) var->Write(*mcOutDir);
    std::cout << "Saving Histos to Data Files" << std::endl;
   
    //for(auto& study: data_studies) study->SaveOrDraw(*data_MichelStudies); 
    // Writing Sidebands now 
    //
    for(auto& study: sideband_studies) study->SaveOrDraw(*mc_SidebandStudies);
    std::cout << "WRiting VARS to event level file" << std::endl;
    for(auto& var: sidevars) var->WriteMC(*mcSideDir);
    std::cout << "WRiting 2D VARS to event level file " << std::endl;
    for(auto& var: sidevars2D) var->Write(*mcSideDir);
    std::cout << "Saving Histos to Data Files" << std::endl;


    //CVUniverse::SetTruth(false);
    //LoopAndFillData(options.m_data, data_band, vars, vars2D, data_studies,sidevars, sidevars2D, data_sidebands, mycuts);
    //std::cout << "Data cut summary:\n" << mycuts << "\n";
    //mycuts.resetStats();

    for(auto& study: data_studies) study->SaveOrDraw(*data_MichelStudies);
    for(auto& study: data_sidebands) study->SaveOrDraw(*data_SidebandStudies);


    //Protons On Target
   
    std::cout << "Printing POT MC " << std::endl;
    auto mcPOT = new TParameter<double>("POTUsed", options.m_mc_pot);
    mcPOT->Write();
    mc_MichelStudies->cd();
    mcPOT->Write();
    mcOutDir->cd();
    mcPOT->Write();
    
    mcSideDir->cd();
    mcPOT->Write();
    

    PlotUtils::TargetUtils targetInfo;
    assert(error_bands["cv"].size() == 1 && "List of error bands must contain a universe named \"cv\" for the flux integral.");
    std::cout << "Looping over Vars to fill systematics" << std::endl;
    for(const auto& var: vars)
    {
      //Flux integral only if systematics are being done (temporary solution)
      util::GetFluxIntegral(*error_bands["cv"].front(), var->efficiencyNumerator->hist)->Write((var->GetName() + "_reweightedflux_integrated").c_str());
      //Always use MC number of nucleons for cross section
      auto nNucleons = new TParameter<double>((var->GetName() + "_fiducial_nucleons").c_str(), targetInfo.GetTrackerNNucleons(minZ, maxZ, true, apothem));
      nNucleons->Write();
    }
    std::cout << "Writing Data Results" << std::endl;
    //Write data results
    TFile* dataOutDir = TFile::Open(DATA_OUT_FILE_NAME, "RECREATE");
    TFile* dataSideDir = TFile::Open(DATA_SIDE_FILE_NAME, "RECREATE");
    if(!dataOutDir)
    {
      std::cerr << "Failed to open a file named " << DATA_OUT_FILE_NAME << " in the current directory for writing histograms.\n";
      return badOutputFile;
    }
    if(!dataSideDir)
    {
      std::cerr << "Failed to open a file named " << DATA_SIDE_FILE_NAME << " in the current directory for writing histograms.\n";
      return badOutputFile;
    }

    std::cout << "Writing Data Vars" << std::endl;
    for(auto& var: vars) var->WriteData(*dataOutDir);
    for(auto& var: vars2D) var->Write(*dataOutDir);
    for(auto& var: sidevars) var->WriteData(*dataSideDir);
    for(auto& var: sidevars2D) var->Write(*dataSideDir); 
   //Protons On Target
    auto dataPOT = new TParameter<double>("POTUsed", options.m_data_pot);
    dataPOT->Write();
    data_MichelStudies->cd();
    dataPOT->Write();
    dataOutDir->cd();
    dataPOT->Write();
    dataSideDir->cd();
    dataPOT->Write();
    dataSideDir->cd();
    std::cout << "Success" << std::endl;
  }
  catch(const ROOT::exception& e)
  {
    std::cerr << "Ending on a ROOT error message.  No histograms will be produced.\n"
              << "If the message talks about \"TNetXNGFile\", this could be a problem with dCache.  The message is:\n"
              << e.what() << "\n" << USAGE << "\n";
    return badFileRead;
  }

  return success;
}
