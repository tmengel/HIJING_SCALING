// Compares per-tower calibrated energy distributions (good towers only) for
// all three calorimeters, before vs after the no-noise waveform fit. The two
// input files are produced by test_waveform.sh, which runs
// Fun4All_UEScaling_Pass1.C twice on the same events:
//   do_waveform_fit=false -> infile1 : read the existing, already-built
//       calibrated towers directly from DST_CALO_CLUSTER (no waveform sim)
//   do_waveform_fit=true  -> infile2 : rebuild towers from G4Hits through
//       CaloWaveformSim (NOISE_NONE) + CaloTowerBuilder (TEMPLATE fit)

#include <sPhenixStyle.C>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TSystem.h>

#include <vector>
#include <string>
#include <iostream>

int plot_tower_energy_comp(
    const std::string & infile1 = "CALO_TREE_nominal_sHijing_0_20fm-00000031-00000.root",
    const std::string & infile2 = "CALO_TREE_noNoise_sHijing_0_20fm-00000031-00000.root",
    const std::string & outdir  = "plots"
)
{
    SetsPhenixStyle();
    gStyle -> SetOptStat( 0 );
    gStyle -> SetOptFit( 0 );

    gSystem -> Exec( Form( "mkdir -p %s", outdir.c_str() ) );

    TFile * f1 = TFile::Open( infile1.c_str(), "READ" );
    TFile * f2 = TFile::Open( infile2.c_str(), "READ" );
    if ( !f1 || f1 -> IsZombie() || !f2 || f2 -> IsZombie() )
    {
        std::cout << "plot_tower_energy_comp: cannot open input file(s)" << std::endl;
        return 1;
    }
    TTree * t1 = (TTree*) f1 -> Get( "T" );
    TTree * t2 = (TTree*) f2 -> Get( "T" );

    const std::vector< std::string > tower_types = { "cemc", "hcalin", "hcalout" };

    std::vector<float> * energy1 = nullptr;
    std::vector<int>   * status1 = nullptr;
    std::vector<float> * energy2 = nullptr;
    std::vector<int>   * status2 = nullptr;

    for ( const auto & det : tower_types )
    {
        t1 -> SetBranchAddress( ( det + "_tower_energy" ).c_str(), &energy1 );
        t1 -> SetBranchAddress( ( det + "_tower_status" ).c_str(), &status1 );
        t2 -> SetBranchAddress( ( det + "_tower_energy" ).c_str(), &energy2 );
        t2 -> SetBranchAddress( ( det + "_tower_status" ).c_str(), &status2 );

        // TH1F * h1 = new TH1F( ( det + "_before" ).c_str(), ";Tower energy [GeV];Towers", 100, -1.0, 5.0 );
        // TH1F * h2 = new TH1F( ( det + "_after" ).c_str(), "", 100, -1.0, 5.0 );
        TH1F * h1 = nullptr;
        TH1F * h2 = nullptr;
        if ( det == "cemc" )
        {
            h1 = new TH1F( ( det + "_before" ).c_str(), ";Tower energy [GeV];Towers", 100, -1.0, 5.0 );
            h2 = new TH1F( ( det + "_after" ).c_str(), "", 100, -1.0, 5.0 );
        }
        else if ( det == "hcalin" )
        {
            h1 = new TH1F( ( det + "_before" ).c_str(), ";Tower energy [GeV];Towers", 100, -0.1, 1.5 );
            h2 = new TH1F( ( det + "_after" ).c_str(), "", 100, -0.1, 1.5 );
        }
        else if ( det == "hcalout" )
        {
            h1 = new TH1F( ( det + "_before" ).c_str(), ";Tower energy [GeV];Towers", 100, -0.1, 1.5 );
            h2 = new TH1F( ( det + "_after" ).c_str(), "", 100, -0.1, 1.5 );
        }

        double sum1 = 0, sum2 = 0;
        int n1 = 0, n2 = 0;

        Long64_t nentries1 = t1 -> GetEntries();
        for ( Long64_t ievt = 0; ievt < nentries1; ++ievt )
        {
            t1 -> GetEntry( ievt );
            for ( size_t ich = 0; ich < energy1 -> size(); ++ich )
            {
                if ( status1 -> at( ich ) != 1 )
                {
                    continue; // only good towers
                }
                h1 -> Fill( energy1 -> at( ich ) );
                sum1 += energy1 -> at( ich );
                n1++;
            }
        }

        Long64_t nentries2 = t2 -> GetEntries();
        for ( Long64_t ievt = 0; ievt < nentries2; ++ievt )
        {
            t2 -> GetEntry( ievt );
            for ( size_t ich = 0; ich < energy2 -> size(); ++ich )
            {
                if ( status2 -> at( ich ) != 1 )
                {
                    continue; // only good towers
                }
                h2 -> Fill( energy2 -> at( ich ) );
                sum2 += energy2 -> at( ich );
                n2++;
            }
        }

        std::cout << det << ": before (nominal) sum=" << sum1 << " GeV over " << n1
                   << " good towers (" << nentries1 << " events), mean="
                   << ( n1 > 0 ? sum1 / n1 : 0.0 ) << " GeV" << std::endl;
        std::cout << det << ": after (no-noise fit) sum=" << sum2 << " GeV over " << n2
                   << " good towers (" << nentries2 << " events), mean="
                   << ( n2 > 0 ? sum2 / n2 : 0.0 ) << " GeV" << std::endl;

        h1 -> SetLineColor( kGray + 2 );
        h1 -> SetFillColorAlpha( kGray, 0.5 );
        h2 -> SetLineColor( kAzure + 2 );
        h2 -> SetFillColorAlpha( kAzure + 2, 0.35 );

        TCanvas * cv = new TCanvas( ( "cv_" + det ).c_str(), "", 700, 600 );
        cv -> SetLogy();
        h1 -> SetTitle( Form( "%s good-tower energy: before vs after no-noise waveform fit", det.c_str() ) );
        h1 -> Draw( "HIST" );
        h2 -> Draw( "HIST SAME" );

        TLegend * leg = new TLegend( 0.45, 0.72, 0.88, 0.86 );
        leg -> AddEntry( h1, "before (nominal)", "f" );
        leg -> AddEntry( h2, "after (no-noise fit)", "f" );
        leg -> Draw();

        cv -> SaveAs( Form( "%s/tower_energy_comp_%s.png", outdir.c_str(), det.c_str() ) );
        cv -> SaveAs( Form( "%s/tower_energy_comp_%s.pdf", outdir.c_str(), det.c_str() ) );
    }

    return 0;
}
