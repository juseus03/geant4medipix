//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: example.cc 69223 2013-04-23 12:36:10Z gcosmo $
//
/// \file G4Medipix.cc
/// \brief Main program of the  example

#include "DetectorConstructionDefault.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include "PrimaryGeneratorMessenger.hh"

#include "G4RunManagerFactory.hh"

#include "G4UImanager.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace {
    void PrintUsage() {
        G4cerr << " Usage: " << G4endl;
        G4cerr << " example [-m macro ] [-u UIsession] [-t nThreads]" << G4endl;
        G4cerr << "   note: -t option is available only for multi-threaded mode."
               << G4endl;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char **argv) {
    // Evaluate arguments
    //
    if (argc > 7) {
        PrintUsage();
        return 1;
    }

    G4String macro;
    G4String session;
#ifdef G4MULTITHREADED
    G4int nThreads = 4;
#endif
    for (G4int i = 1; i < argc; i = i + 2) {
        if (G4String(argv[i]) == "-m") macro = argv[i + 1];
        else if (G4String(argv[i]) == "-u") session = argv[i + 1];
#ifdef G4MULTITHREADED
            else if (G4String(argv[i]) == "-t") {
                nThreads = G4UIcommand::ConvertToInt(argv[i + 1]);
            }
#endif
        else {
            PrintUsage();
            return 1;
        }
    }

    // Choose the Random engine
    //
    CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine());
    G4int seed = (G4int) time(NULL);
    // Possibility to set an offset to random number via environment variable "JOB_ID"
    char *jobID = getenv("JOB_ID");
    if (jobID != nullptr)
        seed += atoi(jobID) * 4852234 * G4UniformRand();
    G4cout << "Seed:" << seed << G4endl;
    G4Random::setTheSeed(seed);

    // Construct the default run manager
    //
    auto runManager = G4RunManagerFactory::CreateRunManager(); 
#ifdef G4MULTITHREADED
    G4cout << "Set number of threads "<<nThreads<<G4endl;
    runManager->SetNumberOfThreads(nThreads);
#endif

    // Set mandatory initialization classes
    //
    runManager->SetUserInitialization(new DetectorConstructionDefault);
    runManager->SetUserInitialization(new PhysicsList);
    runManager->SetUserInitialization(new ActionInitialization);


    PrimaryGeneratorMessenger::GetInstance();

    // Get the pointer to the User Interface manager
    G4UImanager *UImanager = G4UImanager::GetUIpointer();
	
    G4VisManager* visManager = nullptr;

    if (!macro.empty()) {
        // batch mode
        G4String command = "/control/execute ";
        G4cout<<"Batch mode ..."<<G4endl;
        UImanager->ApplyCommand(command + macro);
    } else {
        // interactive mode : define UI session
        G4cout<<"Interactive mode ..."<<G4endl;
        runManager->Initialize();


        G4UIExecutive *ui = new G4UIExecutive(argc, argv, session);

        visManager = new G4VisExecutive("Quiet");
        visManager->Initialize();

        UImanager->ApplyCommand("/control/execute init_vis.mac");

        if (ui->IsGUI())
            UImanager->ApplyCommand("/control/execute gui.mac");
        ui->SessionStart();
        delete ui;
    }

    // Job termination
    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager, so they should not be deleted
    // in the main() program !

    delete visManager;
    delete runManager;
    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
