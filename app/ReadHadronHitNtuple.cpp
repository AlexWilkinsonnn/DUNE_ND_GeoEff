// ROOT includes
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TH3.h>
#include <TCut.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <THStack.h>
#include <TFitResultPtr.h>
#include <TChain.h>
#include <TChainElement.h>
#include <TEfficiency.h>
#include <TMath.h>
#include "TLorentzVector.h"
#include <TRandom3.h>
#include "TSystem.h"
#include "TROOT.h"
#include <TGraph2D.h>
#include <TRandom.h>
#include <TF2.h>

// C++ includes
#include <iostream>
#include <iomanip>
using namespace std;
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <vector> // Need this for generate dictionary for nested vectors
using namespace std;

// Include customized functions and constants
#include "Helpers.h"

float vSize = 30.;

// Draw ND hadronic hits plot
void ReadHadronHitNtuple_ND()
{
  gROOT->Reset();

  // Input FDroot file
  TString FileIn = "/dune/app/users/flynnguo/NDEff/DUNE_ND_GeoEff/bin/Output_FDGeoEff_hadron_61454381.root";
  //
  // Read branch from input trees
  //
  // Read effValues
  TChain *t_effValues = new TChain("effValues");
  t_effValues->Add(FileIn.Data());
  Int_t iwritten;
  Double_t ND_LAr_dtctr_pos;
  Double_t ND_LAr_vtx_pos;
  Double_t ND_GeoEff;
  t_effValues->SetBranchAddress("iwritten",         &iwritten);
  t_effValues->SetBranchAddress("ND_LAr_dtctr_pos",   &ND_LAr_dtctr_pos);
  t_effValues->SetBranchAddress("ND_LAr_vtx_pos",       &ND_LAr_vtx_pos);
  t_effValues->SetBranchAddress("ND_GeoEff",   &ND_GeoEff);
  //hardon info
  TChain *t_effTree = new TChain("effTreeND");
  t_effTree->Add(FileIn.Data());
  int ND_Sim_n_hadronic_Edep_a;
  vector<float> *HadronHitEdeps =0; // Hadron hit segment energy deposits [MeV]
  vector<vector<float>> *CurrentThrowDepsX = 0; // Coordinates of hadron hits X after random throws
  vector<vector<float>> *CurrentThrowDepsY =0; // Coordinates of hadron hits Y after random throws
  vector<vector<float>> *CurrentThrowDepsZ = 0; // Coordinates of hadron hits Z after random throws
  vector<float> *CurrentThrowVetoE = 0;
  vector<float> *CurrentThrowTotE = 0;
  vector<vector<float>> *ND_OffAxis_Sim_hadronic_hit_xyz=0; // coordinates of hadron hits before random throws

  t_effTree->SetBranchAddress("ND_Sim_n_hadronic_Edep_a",         &ND_Sim_n_hadronic_Edep_a);
  t_effTree->SetBranchAddress("CurrentThrowDepsX",         &CurrentThrowDepsX);
  t_effTree->SetBranchAddress("CurrentThrowDepsY",         &CurrentThrowDepsY);
  t_effTree->SetBranchAddress("CurrentThrowDepsZ",         &CurrentThrowDepsZ);
  t_effTree->SetBranchAddress("CurrentThrowVetoE",         &CurrentThrowVetoE);
  t_effTree->SetBranchAddress("CurrentThrowTotE",          &CurrentThrowTotE);
  t_effTree->SetBranchAddress("HadronHitEdeps",            &HadronHitEdeps);
  t_effTree->SetBranchAddress("ND_OffAxis_Sim_hadronic_hit_xyz",            &ND_OffAxis_Sim_hadronic_hit_xyz);

  // Read PosVec
  TChain *t_PosVec = new TChain("PosVec");
  t_PosVec->Add(FileIn.Data());
  vector<Double_t> *ND_LAr_dtctr_pos_vec = 0; // unit: cm, ND off-axis choices for each FD evt
  vector<Double_t> *ND_vtx_vx_vec = 0;       // unit: cm, vtx x choices for each FD evt in ND volume
  vector<Int_t> *iwritten_vec = 0;
  TBranch *b_ND_LAr_dtctr_pos_vec = 0;
  TBranch *b_ND_vtx_vx_vec = 0;
  TBranch *b_iwritten_vec = 0;
  t_PosVec->SetBranchAddress("ND_LAr_dtctr_pos_vec",         &ND_LAr_dtctr_pos_vec , &b_ND_LAr_dtctr_pos_vec);
  t_PosVec->SetBranchAddress("ND_vtx_vx_vec",             &ND_vtx_vx_vec,      &b_ND_vtx_vx_vec);
  t_PosVec->SetBranchAddress("iwritten_vec",               &iwritten_vec,        &b_iwritten_vec);



  Long64_t tentry = t_PosVec->LoadTree(0);
  b_ND_LAr_dtctr_pos_vec->GetEntry(tentry);
  b_ND_vtx_vx_vec->GetEntry(tentry);
  b_iwritten_vec->GetEntry(tentry);

  Int_t ND_LAr_dtctr_pos_vec_size = ND_LAr_dtctr_pos_vec->size();
  Int_t ND_vtx_vx_vec_size = ND_vtx_vx_vec->size();
  Int_t iwritten_vec_size = iwritten_vec->size();
  Int_t tot_size = ND_LAr_dtctr_pos_vec_size*ND_vtx_vx_vec_size;
  Int_t hadronhit_n_plots = tot_size * N_throws;

  Int_t nentries = t_effValues->GetEntries();

  // Output
  // TFile * outFile = new TFile("HadronHitPlots_61454381.root", "RECREATE");
  // TDirectory *IPhadronhit =(TDirectory*)outFile->mkdir("hadron hits"); //create a new folder in the root file
  // TDirectory *IPhadronhit_offaxis =(TDirectory*)outFile->mkdir("OffAxis hadron hits"); //create a new folder in the root file


  // Canvas

  TCanvas** c_hadronhit = new TCanvas*[hadronhit_n_plots];
  TH2F** h_hadronhit_xy = new TH2F*[hadronhit_n_plots];
  TH2F** h_hadronhit_zx = new TH2F*[hadronhit_n_plots];
  TH2F** h_hadronhit_zy = new TH2F*[hadronhit_n_plots];


  TCanvas** c_offaxis_hadronhit = new TCanvas*[tot_size];
  TH2F** h_offaxis_hadronhit_xy = new TH2F*[tot_size];
  TH2F** h_offaxis_hadronhit_zx = new TH2F*[tot_size];
  TH2F** h_offaxis_hadronhit_zy = new TH2F*[tot_size];

  // Set Palette
  gStyle->SetPalette(55);
  gStyle->SetOptStat(110110);
  gStyle->SetStatX(0.85);		//Stat box x position (top right hand corner)
  gStyle->SetStatY(0.9); 		//Stat box y position
  gStyle->SetStatW(0.2);	 		//Stat box width as fraction of pad size
  gStyle->SetStatH(0.15);	 		//Size of each line in stat box
  // gStyle->SetStatFontSize(0.02);	 		//Size of each line in stat box
  // vector<Int_t> a_ND_off_axis_pos_vec = {-2800, -1600, 0};
  vector<Int_t> a_ND_off_axis_pos_vec = {0};
  vector<Int_t> a_ND_vtx_vx_vec = {-299, -292, -285, -278, -271, -216, -24, 24, 216, 264, 271, 278, 285, 292, 299};
  // Store info
  ofstream myfile;
   myfile.open ("Output_HadronhitCheck_61454381.txt");

  // Loop all events
  for (Int_t i_iwritten : *iwritten_vec)
  {
    // if (i_iwritten != 1) continue;
    if(myfileVerbose) myfile<< "i_iwritten: " << i_iwritten << "\n\n";
    cout << "i_iwritten: " << i_iwritten << "\n";
    Int_t canvas_counter = 0;
    for (Int_t i_ND_LAr_dtctr_pos: a_ND_off_axis_pos_vec)
    {
      for (Int_t i_ND_LAr_vtx_pos: a_ND_vtx_vx_vec)
      {
        Int_t i_entry = tot_size * i_iwritten;
        for (i_entry ; i_entry < tot_size * (i_iwritten+1); i_entry++ )
        {
          t_effTree->GetEntry(i_entry);
          t_effValues->GetEntry(i_entry);
          if (ND_LAr_dtctr_pos == i_ND_LAr_dtctr_pos && ND_LAr_vtx_pos == i_ND_LAr_vtx_pos)
          {
            Int_t offset_X = i_ND_LAr_dtctr_pos;
            vector<float> ND_OffAxis_hadronic_hit_xyz;
            vector<vector<float>> ND_OffAxis_hadronic_hit;

            ND_OffAxis_hadronic_hit.clear();
            int it_hadronhit_counter =0;
            for (vector<vector<float>>::iterator it_hadronhit = ND_OffAxis_Sim_hadronic_hit_xyz->begin(); it_hadronhit!=ND_OffAxis_Sim_hadronic_hit_xyz->end(); ++it_hadronhit)
            {
              for (Int_t i = 0; i < 3; i++)
              {
                if(verbose) cout << "it_hadronhit_counter: " << it_hadronhit_counter << ", it_xyz_counter: " << i << ", hit_x,y,z: " << it_hadronhit->at(i) << endl;
                ND_OffAxis_hadronic_hit_xyz.emplace_back(it_hadronhit->at(i));
              }
              ND_OffAxis_hadronic_hit.emplace_back(ND_OffAxis_hadronic_hit_xyz);
              ND_OffAxis_hadronic_hit_xyz.clear();
              it_hadronhit_counter++;
            }

            TString h_hadronhit_xy_name = Form("hadronhitXY_event_%d_OffAxis_%d_cm_LAr_%d_cm", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos);
            h_offaxis_hadronhit_xy[canvas_counter] = new TH2F(h_hadronhit_xy_name,h_hadronhit_xy_name,200,-500,500,200,-500,500);
            h_offaxis_hadronhit_xy[canvas_counter]->GetXaxis()->SetTitle("X [cm]");
            h_offaxis_hadronhit_xy[canvas_counter]->GetYaxis()->SetTitle("Y [cm]");
            h_offaxis_hadronhit_xy[canvas_counter]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

            TString h_hadronhit_zx_name = Form("hadronhitZX_event_%d_OffAxis_%d_cm_LAr_%d_cm", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos);
            h_offaxis_hadronhit_zx[canvas_counter] = new TH2F(h_hadronhit_zx_name,h_hadronhit_zx_name,200,-100,600,200,-500,500);
            h_offaxis_hadronhit_zx[canvas_counter]->GetXaxis()->SetTitle("Z [cm]");
            h_offaxis_hadronhit_zx[canvas_counter]->GetYaxis()->SetTitle("X [cm]");
            h_offaxis_hadronhit_zx[canvas_counter]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

            TString h_hadronhit_zy_name = Form("hadronhitZY_event_%d_OffAxis_%d_cm_LAr_%d_cm", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos);
            h_offaxis_hadronhit_zy[canvas_counter] = new TH2F(h_hadronhit_zy_name,h_hadronhit_zy_name,200,-100,600,200,-500,500);
            h_offaxis_hadronhit_zy[canvas_counter]->GetXaxis()->SetTitle("Z [cm]");
            h_offaxis_hadronhit_zy[canvas_counter]->GetYaxis()->SetTitle("Y [cm]");
            h_offaxis_hadronhit_zy[canvas_counter]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");


            Double_t vetoEnergyND = 0.;
            Double_t totEnergyND = 0.;

            for(Int_t ihadronhit = 0; ihadronhit < ND_OffAxis_hadronic_hit.size(); ihadronhit++)
            {
              h_offaxis_hadronhit_xy[canvas_counter]->Fill(ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X,ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1],HadronHitEdeps->at(ihadronhit));
              h_offaxis_hadronhit_zx[canvas_counter]->Fill(ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2],ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X,HadronHitEdeps->at(ihadronhit));
              h_offaxis_hadronhit_zy[canvas_counter]->Fill(ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2],ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1],HadronHitEdeps->at(ihadronhit));

              totEnergyND += HadronHitEdeps->at(ihadronhit);;
              if ( ( ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X                      > NDActiveVol_min[0]         && ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X                      < NDActiveVol_min[0] + vSize ) ||
                   ( ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1]        > NDActiveVol_min[1]         && ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1]        < NDActiveVol_min[1] + vSize ) ||
                   ( ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2]        > NDActiveVol_min[2]         && ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2]        < NDActiveVol_min[2] + vSize ) ||
                   ( ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X                      > NDActiveVol_max[0] - vSize && ND_OffAxis_hadronic_hit[ihadronhit][0]-offset_X                      < NDActiveVol_max[0] ) ||
                   ( ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1]        > NDActiveVol_max[1] - vSize && ND_OffAxis_hadronic_hit[ihadronhit][1]-NDLAr_OnAxis_offset[1]        < NDActiveVol_max[1] ) ||
                   ( ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2]        > NDActiveVol_max[2] - vSize && ND_OffAxis_hadronic_hit[ihadronhit][2]-NDLAr_OnAxis_offset[2]        < NDActiveVol_max[2] )
                 ){
                   vetoEnergyND += HadronHitEdeps->at(ihadronhit);
              } // end if hadron deposit in FD veto region
              // add outputs
              if(myfileVerbose)
              {
                myfile << "ND_LAr_dtctr_pos: " << ND_LAr_dtctr_pos << ", ND_LAr_vtx_pos: " << ND_LAr_vtx_pos << ", ihadronhit: " << ihadronhit << "\n";
                myfile << "ND_OffAxis_hadronic_hit_X: " << ND_OffAxis_hadronic_hit[ihadronhit][0] - offset_X << "\n";
                myfile << "ND_OffAxis_hadronic_hit_Y: " << ND_OffAxis_hadronic_hit[ihadronhit][1] - NDLAr_OnAxis_offset[1] << "\n";
                myfile << "ND_OffAxis_hadronic_hit_Z: " << ND_OffAxis_hadronic_hit[ihadronhit][2] - NDLAr_OnAxis_offset[2] << "\n\n";
              }
            }
            TString energy_name = Form("VetoE_%.2f_MeV, TotE_%.2f_MeV", vetoEnergyND, totEnergyND);

            // create canvas
            TString c_hadronhit_name = Form("c_hadronhit_event_%d_OffAxis_%d_cm_LAr_%d_cm", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos);
            TString c_hadronhit_title = Form("hadronhit event_%d_OffAxis_%d_cm_LAr_%d_cm", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos);
            c_offaxis_hadronhit[canvas_counter] = new TCanvas(c_hadronhit_name, c_hadronhit_title, 0,53,995,597);
            c_offaxis_hadronhit[canvas_counter]->Clear();
            c_offaxis_hadronhit[canvas_counter]->SetLeftMargin(0.10);
            c_offaxis_hadronhit[canvas_counter]->SetRightMargin(0.10);
            c_offaxis_hadronhit[canvas_counter]->Divide(2,2);

            c_offaxis_hadronhit[canvas_counter]->cd(1);
            c_offaxis_hadronhit[canvas_counter]->GetPad(1)->SetRightMargin(.15);
            h_offaxis_hadronhit_xy[canvas_counter]->Draw("COLZ");
            auto *xy_box = new TBox(NDActiveVol_min[0],NDActiveVol_min[1],NDActiveVol_max[0],NDActiveVol_max[1]);
            xy_box->SetLineColor(kBlack);
            xy_box->SetLineWidth(2);
            xy_box->SetFillStyle(0);
            xy_box->Draw();
            auto *xy_box1 = new TBox(NDActiveVol_min[0]+30,NDActiveVol_min[1]+30,NDActiveVol_max[0]-30,NDActiveVol_max[1]-30);
            xy_box1->SetLineColor(kBlue);
            xy_box1->SetLineWidth(2);
            xy_box1->SetFillStyle(0);
            xy_box1->Draw();
            auto *xy_box2 = new TBox(ND_FV_min[0],ND_FV_min[1],ND_FV_max[0],ND_FV_max[1]);
            xy_box2->SetLineColor(kRed);
            xy_box2->SetLineWidth(2);
            xy_box2->SetFillStyle(0);
            xy_box2->Draw();
            TLatex xy_text(-400,400,energy_name);
            xy_text.DrawClone();

            c_offaxis_hadronhit[canvas_counter]->cd(2);
            c_offaxis_hadronhit[canvas_counter]->GetPad(2)->SetRightMargin(.15);
            h_offaxis_hadronhit_zx[canvas_counter]->Draw("COLZ");
            // ND active volume
            auto *zx_box = new TBox(NDActiveVol_min[2],NDActiveVol_min[0],NDActiveVol_max[2],NDActiveVol_max[0]);
            zx_box->SetLineColor(kBlack);
            zx_box->SetLineWidth(2);
            zx_box->SetFillStyle(0);
            zx_box->Draw();
            // ND veto volume
            auto *zx_box1 = new TBox(NDActiveVol_min[2]+30,NDActiveVol_min[0]+30,NDActiveVol_max[2]-30,NDActiveVol_max[0]-30);
            zx_box1->SetLineColor(kBlue);
            zx_box1->SetLineWidth(2);
            zx_box1->SetFillStyle(0);
            zx_box1->Draw();
            // ND fiducial volume
            auto *zx_box2 = new TBox(ND_FV_min[2],ND_FV_min[0],ND_FV_max[2],ND_FV_max[0]);
            zx_box2->SetLineColor(kRed);
            zx_box2->SetLineWidth(2);
            zx_box2->SetFillStyle(0);
            zx_box2->Draw();
            TLatex xz_text(-50,400,energy_name);
            xz_text.DrawClone();

            c_offaxis_hadronhit[canvas_counter]->cd(3);
            c_offaxis_hadronhit[canvas_counter]->GetPad(3)->SetRightMargin(.15);
            h_offaxis_hadronhit_zy[canvas_counter]->Draw("COLZ");
            auto *yz_box = new TBox(NDActiveVol_min[2],NDActiveVol_min[1],NDActiveVol_max[2],NDActiveVol_max[1]);
            yz_box->SetLineColor(kBlack);
            yz_box->SetLineWidth(2);
            yz_box->SetFillStyle(0);
            yz_box->Draw();
            auto *yz_box1 = new TBox(NDActiveVol_min[2]+30,NDActiveVol_min[1]+30,NDActiveVol_max[2]-30,NDActiveVol_max[1]-30);
            yz_box1->SetLineColor(kBlue);
            yz_box1->SetLineWidth(2);
            yz_box1->SetFillStyle(0);
            yz_box1->Draw();
            auto *yz_box2 = new TBox(ND_FV_min[2],ND_FV_min[1],ND_FV_max[2],ND_FV_max[1]);
            yz_box2->SetLineColor(kRed);
            yz_box2->SetLineWidth(2);
            yz_box2->SetFillStyle(0);
            yz_box2->Draw();
            TLatex yz_text(-50,400,energy_name);
            yz_text.DrawClone();


            gPad->Update();
            gPad->Modified();
            gSystem->ProcessEvents();


            // outFile->cd("OffAxis hadron hits");
            // c_offaxis_hadronhit[canvas_counter]->Write();
            c_offaxis_hadronhit[canvas_counter]->SaveAs( TString::Format("HadronHitPlots/c_event_%d_onaxis_hadronhit_%d.pdf",iwritten, canvas_counter ) );
            // c_offaxis_hadronhit[canvas_counter]->Close();

            if(verbose) cout << "canvas_counter: " << canvas_counter << ", offset_X: " << offset_X << endl;
            canvas_counter++;
          }
        }
      }
    }
    //
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //
    // Create canvas for hadron hit
    Int_t n_plot = 0;
    Int_t i_n_plot = 0;
    for (Int_t i_ND_LAr_dtctr_pos: a_ND_off_axis_pos_vec)
    {
      for (Int_t i_ND_LAr_vtx_pos: a_ND_vtx_vx_vec)
      {
        Int_t i_entry = tot_size * i_iwritten;
        for (i_entry ; i_entry < tot_size * (i_iwritten+1); i_entry++ )
        {
          t_effTree->GetEntry(i_entry);
          t_effValues->GetEntry(i_entry);

          if (ND_LAr_dtctr_pos == i_ND_LAr_dtctr_pos && ND_LAr_vtx_pos == i_ND_LAr_vtx_pos)
          {
            Int_t offset_X = i_ND_LAr_dtctr_pos;
            // Store hadron hit info into vector
            vector<float> ThrowDepsX_hit;
            vector<vector<float>> ThrowDepsX;
            ThrowDepsX.clear();
            int it_throw_x_counter =0;
            for (vector<vector<float>>::iterator it_throw = CurrentThrowDepsX->begin(); it_throw!=CurrentThrowDepsX->end(); ++it_throw)
            {
              for (Int_t ihadronhit =0; ihadronhit < ND_Sim_n_hadronic_Edep_a; ihadronhit++)
              {
                if(verbose) cout << "it_throw_x_counter: " << it_throw_x_counter << ", ihadronhit: " << ihadronhit << ", hit_x: " << it_throw->at(ihadronhit) << endl;
                ThrowDepsX_hit.emplace_back(it_throw->at(ihadronhit));
              }
              ThrowDepsX.emplace_back(ThrowDepsX_hit);
              ThrowDepsX_hit.clear();
              it_throw_x_counter++;
            }
            vector<float> ThrowDepsY_hit;
            vector<vector<float>> ThrowDepsY;
            ThrowDepsY.clear();
            int it_throw_y_counter =0;
            for (vector<vector<float>>::iterator it_throw = CurrentThrowDepsY->begin(); it_throw!=CurrentThrowDepsY->end(); ++it_throw)
            {
              for (Int_t ihadronhit =0; ihadronhit < ND_Sim_n_hadronic_Edep_a; ihadronhit++)
              {
                if(verbose) cout << "it_throw_x_counter: " << it_throw_x_counter << ", ihadronhit: " << ihadronhit << ", hit_y: " << it_throw->at(ihadronhit) << endl;
                ThrowDepsY_hit.emplace_back(it_throw->at(ihadronhit));
              }
              ThrowDepsY.emplace_back(ThrowDepsY_hit);
              ThrowDepsY_hit.clear();
              it_throw_y_counter++;
            }
            vector<float> ThrowDepsZ_hit;
            vector<vector<float>> ThrowDepsZ;
            ThrowDepsZ.clear();
            int it_throw_z_counter =0;
            for (vector<vector<float>>::iterator it_throw = CurrentThrowDepsZ->begin(); it_throw!=CurrentThrowDepsZ->end(); ++it_throw)
            {
              for (Int_t ihadronhit =0; ihadronhit < ND_Sim_n_hadronic_Edep_a; ihadronhit++)
              {
                if(verbose) cout<< "ND_LAr_dtctr_pos: " << ND_LAr_dtctr_pos << ", ND_LAr_vtx_pos: " << ND_LAr_vtx_pos << ", ithrow: " << it_throw_z_counter << ", ihadronhit: " << ihadronhit << ", hit_z: " << it_throw->at(ihadronhit) << endl;
                ThrowDepsZ_hit.emplace_back(it_throw->at(ihadronhit));
              }
              ThrowDepsZ.emplace_back(ThrowDepsZ_hit);
              ThrowDepsZ_hit.clear();
              it_throw_z_counter++;
            }

            // Draw plots
            // for(Int_t ithrow = 0; ithrow < N_throws; ithrow++)
            for(Int_t ithrow = 0; ithrow < 10; ithrow++)
            {
              cout << "ithrow: " << ithrow <<endl;
              n_plot = i_n_plot*N_throws + ithrow;

              TString h_hadronhit_xy_name = Form("hadronhitXY_event_%d_OffAxis_%d_cm_LAr_%d_cm_throw_%d", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos,ithrow);
              h_hadronhit_xy[n_plot] = new TH2F(h_hadronhit_xy_name,h_hadronhit_xy_name,200,-500,500,200,-500,500);
              h_hadronhit_xy[n_plot]->GetXaxis()->SetTitle("X [cm]");
              h_hadronhit_xy[n_plot]->GetYaxis()->SetTitle("Y [cm]");
              h_hadronhit_xy[n_plot]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

              TString h_hadronhit_zx_name = Form("hadronhitZX_event_%d_OffAxis_%d_cm_LAr_%d_cm_throw_%d", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos,ithrow);
              h_hadronhit_zx[n_plot] = new TH2F(h_hadronhit_zx_name,h_hadronhit_zx_name,200,-100,600,200,-500,500);
              h_hadronhit_zx[n_plot]->GetXaxis()->SetTitle("Z [cm]");
              h_hadronhit_zx[n_plot]->GetYaxis()->SetTitle("X [cm]");
              h_hadronhit_zx[n_plot]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

              TString h_hadronhit_zy_name = Form("hadronhitZY_event_%d_OffAxis_%d_cm_LAr_%d_cm_throw_%d", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos,ithrow);
              h_hadronhit_zy[n_plot] = new TH2F(h_hadronhit_zy_name,h_hadronhit_zy_name,200,-100,600,200,-500,500);
              h_hadronhit_zy[n_plot]->GetXaxis()->SetTitle("Z [cm]");
              h_hadronhit_zy[n_plot]->GetYaxis()->SetTitle("Y [cm]");
              h_hadronhit_zy[n_plot]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

              for(Int_t ihadronhit =0; ihadronhit < ND_Sim_n_hadronic_Edep_a; ihadronhit++)
              {
                h_hadronhit_xy[n_plot]->Fill(ThrowDepsX[ithrow][ihadronhit] - offset_X,ThrowDepsY[ithrow][ihadronhit]-NDLAr_OnAxis_offset[1],HadronHitEdeps->at(ihadronhit));
                h_hadronhit_zx[n_plot]->Fill(ThrowDepsZ[ithrow][ihadronhit] - NDLAr_OnAxis_offset[2],ThrowDepsX[ithrow][ihadronhit] - offset_X,HadronHitEdeps->at(ihadronhit));
                h_hadronhit_zy[n_plot]->Fill(ThrowDepsZ[ithrow][ihadronhit] - NDLAr_OnAxis_offset[2],ThrowDepsY[ithrow][ihadronhit] - NDLAr_OnAxis_offset[1],HadronHitEdeps->at(ihadronhit));
                if(myfileVerbose)
                {
                  myfile << "ND_LAr_dtctr_pos: " << ND_LAr_dtctr_pos << ", ND_LAr_vtx_pos: " << ND_LAr_vtx_pos << ", ThrowDepsX at ithrow" << ithrow << ", at ihadronhit: " << ihadronhit << ", is: " << ThrowDepsX[ithrow][ihadronhit] - offset_X<< "\n";
                  myfile << "ND_LAr_dtctr_pos: " << ND_LAr_dtctr_pos << ", ND_LAr_vtx_pos: " << ND_LAr_vtx_pos << ", ThrowDepsY at ithrow" << ithrow << ", at ihadronhit: " << ihadronhit << ", is: " << ThrowDepsY[ithrow][ihadronhit] - NDLAr_OnAxis_offset[1] << "\n";
                  myfile << "ND_LAr_dtctr_pos: " << ND_LAr_dtctr_pos << ", ND_LAr_vtx_pos: " << ND_LAr_vtx_pos << ", ThrowDepsZ at ithrow" << ithrow << ", at ihadronhit: " << ihadronhit << ", is: " << ThrowDepsZ[ithrow][ihadronhit] - NDLAr_OnAxis_offset[2] << "\n\n";
                }
              }
              TString energy_name = Form("VetoE_%.2f_MeV, TotE_%.2f_MeV", CurrentThrowVetoE->at(ithrow), CurrentThrowTotE->at(ithrow));

              TString c_hadronhit_name = Form("c_hadronhit_event_%d_OffAxis_%d_cm_LAr_%d_cm_throw_%d", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos,ithrow);
              TString c_hadronhit_title = Form("hadronhit event_%d_OffAxis_%d_cm_LAr_%d_cm_throw_%d", i_iwritten,i_ND_LAr_dtctr_pos,i_ND_LAr_vtx_pos,ithrow);
              c_hadronhit[n_plot] = new TCanvas(c_hadronhit_name, c_hadronhit_title, 0,53,995,597);
              c_hadronhit[n_plot]->Clear();
              c_hadronhit[n_plot]->Range(-666.6667,-625,1000,625);
              c_hadronhit[n_plot]->SetLeftMargin(0.10);
              c_hadronhit[n_plot]->SetRightMargin(0.10);
              c_hadronhit[n_plot]->Divide(2,2);

              c_hadronhit[n_plot]->cd(1);
              c_hadronhit[n_plot]->GetPad(1)->SetRightMargin(.15);
              h_hadronhit_xy[n_plot]->Draw("COLZ");
              auto *xy_box = new TBox(NDActiveVol_min[0],NDActiveVol_min[1],NDActiveVol_max[0],NDActiveVol_max[1]);
              xy_box->SetLineColor(kBlack);
              xy_box->SetLineWidth(2);
              xy_box->SetFillStyle(0);
              xy_box->Draw();
              auto *xy_box1 = new TBox(NDActiveVol_min[0]+30,NDActiveVol_min[1]+30,NDActiveVol_max[0]-30,NDActiveVol_max[1]-30);
              xy_box1->SetLineColor(kBlue);
              xy_box1->SetLineWidth(2);
              xy_box1->SetFillStyle(0);
              xy_box1->Draw();
              auto *xy_box2 = new TBox(ND_FV_min[0],ND_FV_min[1],ND_FV_max[0],ND_FV_max[1]);
              xy_box2->SetLineColor(kRed);
              xy_box2->SetLineWidth(2);
              xy_box2->SetFillStyle(0);
              xy_box2->Draw();
              TLatex xy_text(-400,400,energy_name);
              xy_text.DrawClone();

              c_hadronhit[n_plot]->cd(2);
              c_hadronhit[n_plot]->GetPad(2)->SetRightMargin(.15);
              h_hadronhit_zx[n_plot]->Draw("COLZ");
              // ND active volume
              auto *zx_box = new TBox(NDActiveVol_min[2],NDActiveVol_min[0],NDActiveVol_max[2],NDActiveVol_max[0]);
              zx_box->SetLineColor(kBlack);
              zx_box->SetLineWidth(2);
              zx_box->SetFillStyle(0);
              zx_box->Draw();
              // ND veto volume
              auto *zx_box1 = new TBox(NDActiveVol_min[2]+30,NDActiveVol_min[0]+30,NDActiveVol_max[2]-30,NDActiveVol_max[0]-30);
              zx_box1->SetLineColor(kBlue);
              zx_box1->SetLineWidth(2);
              zx_box1->SetFillStyle(0);
              zx_box1->Draw();
              // ND fiducial volume
              auto *zx_box2 = new TBox(ND_FV_min[2],ND_FV_min[0],ND_FV_max[2],ND_FV_max[0]);
              zx_box2->SetLineColor(kRed);
              zx_box2->SetLineWidth(2);
              zx_box2->SetFillStyle(0);
              zx_box2->Draw();
              TLatex xz_text(-50,400,energy_name);
              xz_text.DrawClone();

              c_hadronhit[n_plot]->cd(3);
              c_hadronhit[n_plot]->GetPad(3)->SetRightMargin(.15);
              h_hadronhit_zy[n_plot]->Draw("COLZ");
              auto *yz_box = new TBox(NDActiveVol_min[2],NDActiveVol_min[1],NDActiveVol_max[2],NDActiveVol_max[1]);
              yz_box->SetLineColor(kBlack);
              yz_box->SetLineWidth(2);
              yz_box->SetFillStyle(0);
              yz_box->Draw();
              auto *yz_box1 = new TBox(NDActiveVol_min[2]+30,NDActiveVol_min[1]+30,NDActiveVol_max[2]-30,NDActiveVol_max[1]-30);
              yz_box1->SetLineColor(kBlue);
              yz_box1->SetLineWidth(2);
              yz_box1->SetFillStyle(0);
              yz_box1->Draw();
              auto *yz_box2 = new TBox(ND_FV_min[2],ND_FV_min[1],ND_FV_max[2],ND_FV_max[1]);
              yz_box2->SetLineColor(kRed);
              yz_box2->SetLineWidth(2);
              yz_box2->SetFillStyle(0);
              yz_box2->Draw();
              TLatex yz_text(-50,400,energy_name);
              yz_text.DrawClone();



              gPad->Update();
              gPad->Modified();
              gSystem->ProcessEvents();

              // outFile->cd("hadron hits");
              // c_hadronhit[n_plot]->Write();
              c_hadronhit[n_plot]->SaveAs( TString::Format("HadronHitPlots/c_event_%d_hadronhit_%d.pdf",iwritten, n_plot ) );
              // c_hadronhit[n_plot]->Close();

            }
            i_n_plot++;
          }
        }
      }
    }


  } // end iwritten_vec



  delete[] h_hadronhit_xy;
  delete[] h_hadronhit_zx;
  delete[] h_hadronhit_zy;
  delete[] c_hadronhit;
  delete[] h_offaxis_hadronhit_zy;
  delete[] h_offaxis_hadronhit_zx;
  delete[] h_offaxis_hadronhit_xy;
  delete[] c_offaxis_hadronhit;

  myfile.close();
  // outFile->Close();
} // end ReadNtuple

// Draw FD hadronic hits plot
void ReadHadronHitNtuple_FD()
{
  gROOT->Reset();

  // Input FDroot file
  TString FileIn = "/dune/app/users/flynnguo/NDEff/DUNE_ND_GeoEff/app/myntuple_64401080_cut.root";

  TChain *t = new TChain("MyEnergyAnalysis/MyTree");
  t->Add(FileIn.Data());

  double FD_LepE;                      // Generator level neutrino lepton energy
  double FD_E_vis_true;                 // True vis energy [GeV]
  t->SetBranchAddress("LepE",           &FD_LepE);
  t->SetBranchAddress("E_vis_true",     &FD_E_vis_true);
  // Muon
  double FD_Gen_numu_E;                // Energy of generator level neutrino [GeV]
  double FD_Sim_mu_start_vx;           // x position of the muon trajectory start
  double FD_Sim_mu_start_vy;           // y .....................................
  double FD_Sim_mu_start_vz;           // z .....................................
  double FD_Sim_mu_end_vx;             // x position of the muon trajectory end
  double FD_Sim_mu_end_vy;             // y ...................................
  double FD_Sim_mu_end_vz;             // z ...................................
  double FD_Sim_mu_Edep_a1;                // muon energy deposit [GeV]: total amount of electrons reaching the readout channel
  double FD_Sim_mu_Edep_a2;                // muon energy deposit [MeV]: total amount of energy released by ionizations in the event (from Geant4 simulation)
  double FD_Sim_mu_Edep_b1;                // [GeV]
  double FD_Sim_mu_Edep_b2;                // [MeV]
  t->SetBranchAddress("Gen_numu_E",     &FD_Gen_numu_E);
  t->SetBranchAddress("Sim_mu_start_vx",          &FD_Sim_mu_start_vx);
  t->SetBranchAddress("Sim_mu_start_vy",          &FD_Sim_mu_start_vy);
  t->SetBranchAddress("Sim_mu_start_vz",          &FD_Sim_mu_start_vz);
  t->SetBranchAddress("Sim_mu_end_vx",            &FD_Sim_mu_end_vx);
  t->SetBranchAddress("Sim_mu_end_vy",            &FD_Sim_mu_end_vy);
  t->SetBranchAddress("Sim_mu_end_vz",            &FD_Sim_mu_end_vz);
  t->SetBranchAddress("Sim_mu_Edep_a1",     &FD_Sim_mu_Edep_a1);
  t->SetBranchAddress("Sim_mu_Edep_a2",     &FD_Sim_mu_Edep_a2);
  t->SetBranchAddress("Sim_mu_Edep_b1",     &FD_Sim_mu_Edep_b1);
  t->SetBranchAddress("Sim_mu_Edep_b2",     &FD_Sim_mu_Edep_b2);

  int FD_Sim_nMu; // # of Sim muons (mu+/mu-)
  int FD_CCNC_truth; // 0 =CC 1 =NC
  int FD_neuPDG; // Generator level neutrino PDG
  t->SetBranchAddress("Sim_nMu",                  &FD_Sim_nMu);
  t->SetBranchAddress("CCNC_truth",               &FD_CCNC_truth);
  t->SetBranchAddress("neuPDG",                   &FD_neuPDG);

  // Hardon hits
  int    FD_Sim_n_hadronic_Edep_a;   // # of hadronic energy deposits
  double FD_Sim_hadronic_Edep_a2;    // Total amount of energy released by ionizations in the event (from Geant4 simulation) [MeV]
  vector<float> *FD_Sim_hadronic_hit_Edep_a2 = 0; // Need initialize 0 here to avoid error
  vector<float> *FD_Sim_hadronic_hit_x_a = 0; // Position of each energy deposit on the x-axis [cm]
  vector<float> *FD_Sim_hadronic_hit_y_a = 0; // Position of each energy deposit on the y-axis [cm]
  vector<float> *FD_Sim_hadronic_hit_z_a = 0; // Position of each energy deposit on the z-axis [cm]
  t->SetBranchAddress("Sim_hadronic_Edep_a2",     &FD_Sim_hadronic_Edep_a2);
  t->SetBranchAddress("Sim_n_hadronic_Edep_a",    &FD_Sim_n_hadronic_Edep_a);
  t->SetBranchAddress("Sim_hadronic_hit_Edep_a2", &FD_Sim_hadronic_hit_Edep_a2);
  t->SetBranchAddress("Sim_hadronic_hit_x_a",     &FD_Sim_hadronic_hit_x_a);
  t->SetBranchAddress("Sim_hadronic_hit_y_a",     &FD_Sim_hadronic_hit_y_a);
  t->SetBranchAddress("Sim_hadronic_hit_z_a",     &FD_Sim_hadronic_hit_z_a);

  Int_t nentries = t->GetEntries();
  cout << "nentries: " << nentries << endl;

  TCanvas** c_hadronhit = new TCanvas*[nentries];
  TH2F** h_hadronhit_xy = new TH2F*[nentries];
  TH2F** h_hadronhit_zx = new TH2F*[nentries];
  TH2F** h_hadronhit_zy = new TH2F*[nentries];
  // Set Palette
  gStyle->SetPalette(55);
  gStyle->SetOptStat(0); // Remove Stat Box
  // gStyle->SetOptStat("nemou");
  // gStyle->SetStatX(0.85);		//Stat box x position (top right hand corner)
  // gStyle->SetStatY(0.9); 		//Stat box y position
  // gStyle->SetStatW(0.2);	 		//Stat box width as fraction of pad size
  // gStyle->SetStatH(0.15);	 		//Size of each line in stat box

  vector<float> vetoEnergyFD_v; // Total hadron deposited energy in FD veto region

  // Print out the data
  ofstream myfile;
   myfile.open ("Output_FD_HadronhitCheck_64401080.txt");


  // create histograms
  for ( int ientry = 0; ientry < nentries; ientry++ )
  {
    t->GetEntry(ientry);

    TString h_hadronhit_xy_name = Form("hadronhitXY_event_%d", ientry);
    h_hadronhit_xy[ientry] = new TH2F(h_hadronhit_xy_name,h_hadronhit_xy_name,200,-500,500,200,-800,800);
    h_hadronhit_xy[ientry]->GetXaxis()->SetTitle("X [cm]");
    h_hadronhit_xy[ientry]->GetYaxis()->SetTitle("Y [cm]");
    h_hadronhit_xy[ientry]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

    TString h_hadronhit_zx_name = Form("hadronhitZX_event_%d", ientry);
    h_hadronhit_zx[ientry] = new TH2F(h_hadronhit_zx_name,h_hadronhit_zx_name,200,-100,1500,200,-500,500);
    h_hadronhit_zx[ientry]->GetXaxis()->SetTitle("Z [cm]");
    h_hadronhit_zx[ientry]->GetYaxis()->SetTitle("X [cm]");
    h_hadronhit_zx[ientry]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");

    TString h_hadronhit_zy_name = Form("hadronhitZY_event_%d", ientry);
    h_hadronhit_zy[ientry] = new TH2F(h_hadronhit_zy_name,h_hadronhit_zy_name,200,-100,1500,200,-800,800);
    h_hadronhit_zy[ientry]->GetXaxis()->SetTitle("Z [cm]");
    h_hadronhit_zy[ientry]->GetYaxis()->SetTitle("Y [cm]");
    h_hadronhit_zy[ientry]->GetZaxis()->SetTitle("HadronHitEdeps [MeV]");
  }

  // fill events

  for ( int ientry = 0; ientry < nentries; ientry++ )
  {
    t->GetEntry(ientry);

    if(myfileVerbose)
    {
      myfile<< "ientry: " << ientry << "\n\n";
    }

    float vetoEnergyFD = 0.;
    for(Int_t ihadronhit =0; ihadronhit < FD_Sim_n_hadronic_Edep_a; ihadronhit++)
    {
      // Veto region size: 30 cm from the active volume
      if ( ( FD_Sim_hadronic_hit_x_a->at(ihadronhit) > FDActiveVol_min[0] && FD_Sim_hadronic_hit_x_a->at(ihadronhit) < FDActiveVol_min[0] + 30 ) ||
           ( FD_Sim_hadronic_hit_y_a->at(ihadronhit) > FDActiveVol_min[1] && FD_Sim_hadronic_hit_y_a->at(ihadronhit) < FDActiveVol_min[1] + 30 ) ||
           ( FD_Sim_hadronic_hit_z_a->at(ihadronhit) > FDActiveVol_min[2] && FD_Sim_hadronic_hit_z_a->at(ihadronhit) < FDActiveVol_min[2] + 30 ) ||
           ( FD_Sim_hadronic_hit_x_a->at(ihadronhit) > FDActiveVol_max[0] - 30 && FD_Sim_hadronic_hit_x_a->at(ihadronhit) < FDActiveVol_max[0] ) ||
           ( FD_Sim_hadronic_hit_y_a->at(ihadronhit) > FDActiveVol_max[1] - 30 && FD_Sim_hadronic_hit_y_a->at(ihadronhit) < FDActiveVol_max[1] ) ||
           ( FD_Sim_hadronic_hit_z_a->at(ihadronhit) > FDActiveVol_max[2] - 30 && FD_Sim_hadronic_hit_z_a->at(ihadronhit) < FDActiveVol_max[2] )
         ){
           vetoEnergyFD += FD_Sim_hadronic_hit_Edep_a2->at(ihadronhit);
      } // end if hadron deposit in FD veto region

      h_hadronhit_xy[ientry]->Fill(FD_Sim_hadronic_hit_x_a->at(ihadronhit), FD_Sim_hadronic_hit_y_a->at(ihadronhit),FD_Sim_hadronic_hit_Edep_a2->at(ihadronhit));
      h_hadronhit_zx[ientry]->Fill(FD_Sim_hadronic_hit_z_a->at(ihadronhit), FD_Sim_hadronic_hit_x_a->at(ihadronhit),FD_Sim_hadronic_hit_Edep_a2->at(ihadronhit));
      h_hadronhit_zy[ientry]->Fill(FD_Sim_hadronic_hit_z_a->at(ihadronhit), FD_Sim_hadronic_hit_y_a->at(ihadronhit),FD_Sim_hadronic_hit_Edep_a2->at(ihadronhit));
      if(myfileVerbose)
      {
        myfile << "ientry: " << ientry << ", ihadronhit: " << ihadronhit << "\n";
        myfile << "FD_Sim_hadronic_hit_x_a: " << FD_Sim_hadronic_hit_x_a->at(ihadronhit) << "\n";
        myfile << "FD_Sim_hadronic_hit_y_a: " << FD_Sim_hadronic_hit_y_a->at(ihadronhit) << "\n";
        myfile << "FD_Sim_hadronic_hit_z_a: " << FD_Sim_hadronic_hit_z_a->at(ihadronhit) << "\n\n";
      }
    }
    vetoEnergyFD_v.emplace_back(vetoEnergyFD);
    cout << "ientry: " << ientry << endl;



  }

  // draw histograms
  for ( int ientry = 0; ientry < nentries; ientry++ )
  {
    t->GetEntry(ientry);

    TString energy_name1 = Form("VetoE_%.2f_MeV, TotE_%.2f_MeV, VisTrueE_%.2f_GeV", vetoEnergyFD_v.at(ientry), FD_Sim_hadronic_Edep_a2, FD_E_vis_true);
    TString energy_name2 = Form("LepE_%.2f_GeV, GenNumuE_%.2f_GeV, DepMuE_%.2f_MeV", FD_LepE, FD_Gen_numu_E, FD_Sim_mu_Edep_a2);
    TString c_hadronhit_name = Form("c_hadronhit_event_%d", ientry);
    TString c_hadronhit_title = Form("hadronhit event_%d", ientry);
    c_hadronhit[ientry] = new TCanvas(c_hadronhit_name, c_hadronhit_title, 0,53,995,597);
    c_hadronhit[ientry]->Clear();
    c_hadronhit[ientry]->Range(-666.6667,-625,1000,625);
    c_hadronhit[ientry]->SetLeftMargin(0.10);
    c_hadronhit[ientry]->SetRightMargin(0.10);
    c_hadronhit[ientry]->Divide(2,2);

    c_hadronhit[ientry]->cd(1);
    c_hadronhit[ientry]->GetPad(1)->SetRightMargin(.15);
    h_hadronhit_xy[ientry]->Draw("COLZ");
    auto *xy_box = new TBox(FDActiveVol_min[0],FDActiveVol_min[1],FDActiveVol_max[0],FDActiveVol_max[1]);
    xy_box->SetLineColor(kBlack);
    xy_box->SetLineWidth(1);
    xy_box->SetFillStyle(0);
    xy_box->Draw();
    auto *xy_box1 = new TBox(FDActiveVol_min[0]+vSize,FDActiveVol_min[1]+vSize,FDActiveVol_max[0]-vSize,FDActiveVol_max[1]-vSize);
    xy_box1->SetLineColor(kBlue);
    xy_box1->SetLineWidth(1);
    xy_box1->SetFillStyle(0);
    xy_box1->Draw();
    auto *xy_box2 = new TBox(FD_FV_min[0],FD_FV_min[1],FD_FV_max[0],FD_FV_max[1]);
    xy_box2->SetLineColor(kRed);
    xy_box2->SetLineWidth(1);
    xy_box2->SetFillStyle(0);
    xy_box2->Draw();
    TLatex xy_text(-480,620,energy_name1);
    TLatex xy_text_d(-480,-720,energy_name2);
    xy_text.SetTextSize(0.04);
    xy_text_d.SetTextSize(0.04);
    xy_text.DrawClone();
    xy_text_d.DrawClone();

    c_hadronhit[ientry]->cd(2);
    c_hadronhit[ientry]->GetPad(2)->SetRightMargin(.15);
    h_hadronhit_zx[ientry]->Draw("COLZ");
    // ND active volume
    auto *zx_box = new TBox(FDActiveVol_min[2],FDActiveVol_min[0],FDActiveVol_max[2],FDActiveVol_max[0]);
    zx_box->SetLineColor(kBlack);
    zx_box->SetLineWidth(1);
    zx_box->SetFillStyle(0);
    zx_box->Draw();
    // ND veto volume
    auto *zx_box1 = new TBox(FDActiveVol_min[2]+vSize,FDActiveVol_min[0]+vSize,FDActiveVol_max[2]-vSize,FDActiveVol_max[0]-vSize);
    zx_box1->SetLineColor(kBlue);
    zx_box1->SetLineWidth(1);
    zx_box1->SetFillStyle(0);
    zx_box1->Draw();
    // ND fiducial volume
    auto *zx_box2 = new TBox(FD_FV_min[2],FD_FV_min[0],FD_FV_max[2],FD_FV_max[0]);
    zx_box2->SetLineColor(kRed);
    zx_box2->SetLineWidth(1);
    zx_box2->SetFillStyle(0);
    zx_box2->Draw();
    TLatex xz_text(-50,400,energy_name1);
    TLatex xz_text_d(-50,-480,energy_name2);
    xz_text.SetTextSize(0.04);
    xz_text_d.SetTextSize(0.04);
    xz_text.DrawClone();
    xz_text_d.DrawClone();

    c_hadronhit[ientry]->cd(3);
    c_hadronhit[ientry]->GetPad(3)->SetRightMargin(.15);
    h_hadronhit_zy[ientry]->Draw("COLZ");
    auto *yz_box = new TBox(FDActiveVol_min[2],FDActiveVol_min[1],FDActiveVol_max[2],FDActiveVol_max[1]);
    yz_box->SetLineColor(kBlack);
    yz_box->SetLineWidth(1);
    yz_box->SetFillStyle(0);
    yz_box->Draw();
    auto *yz_box1 = new TBox(FDActiveVol_min[2]+vSize,FDActiveVol_min[1]+vSize,FDActiveVol_max[2]-vSize,FDActiveVol_max[1]-vSize);
    yz_box1->SetLineColor(kBlue);
    yz_box1->SetLineWidth(1);
    yz_box1->SetFillStyle(0);
    yz_box1->Draw();
    auto *yz_box2 = new TBox(FD_FV_min[2],FD_FV_min[1],FD_FV_max[2],FD_FV_max[1]);
    yz_box2->SetLineColor(kRed);
    yz_box2->SetLineWidth(1);
    yz_box2->SetFillStyle(0);
    yz_box2->Draw();
    TLatex yz_text(-50,620,energy_name1);
    TLatex yz_text_d(-50,-720,energy_name2);
    yz_text.SetTextSize(0.04);
    yz_text_d.SetTextSize(0.04);
    yz_text.DrawClone();
    yz_text_d.DrawClone();

    gPad->Update();
    gPad->Modified();
    gSystem->ProcessEvents();

    c_hadronhit[ientry]->SaveAs( TString::Format("HadronHitPlots_FD/c_event_%d_hadronhit.pdf",ientry) );
  }


  delete[] h_hadronhit_xy;
  delete[] h_hadronhit_zx;
  delete[] h_hadronhit_zy;
  delete[] c_hadronhit;

  myfile.close();
}
