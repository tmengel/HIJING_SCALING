#ifndef CaloManip_CaloManip_H
#define CaloManip_CaloManip_H

#include <fun4all/SubsysReco.h>

#include <string>
#include <vector>

class TFile;
class TTree;

class PHCompositeNode;

class CaloManip : public SubsysReco {
 
 public:

  CaloManip( const std::string & infile )
    : SubsysReco("CaloManip")
    , m_input_file( infile )
  {}
  ~CaloManip() override {}

  int Init( PHCompositeNode * /*topNode*/ ) override;
  int process_event( PHCompositeNode * topNode ) override;
  int End( PHCompositeNode * /*topNode*/ ) override;

  void add_event_header ( const std::string & name = "EventHeader" ) { m_eventhead_node = name; }
  void add_cemc_node    ( const std::string & name = "TOWERINFO_CALIB_CEMC_RETOWERED" ) { m_cemc_node = name; }
  void add_hcalin_node  ( const std::string & name = "TOWERINFO_CALIB_HCALIN" ) { m_hcalin_node = name; }
  void add_hcalout_node ( const std::string & name = "TOWERINFO_CALIB_HCALOUT" ) { m_hcalout_node = name; }

  void set_scale_factor ( const float factor ) { m_scale_factor = factor; }

 private:
  
  std::string m_input_file { "" };

  std::string m_eventhead_node { "EventHeader" };

  std::string m_cemc_node { "" };
  std::string m_hcalin_node { "" };
  std::string m_hcalout_node { "" };

  TFile * m_tfile {nullptr};
  TTree * m_ttree {nullptr};
  
  int m_event_id { -1 };
  int m_nentries_in_tree { -1 };
  int m_ttree_runnumber { -1 };
  float m_ttree_b { 0.0 };
  float m_ttree_ep_angle { 0.0 };
  float m_ttree_ecc { 0.0 };
  float m_ttree_ncoll { 0.0 };
  float m_ttree_psi1 { 0.0 };
  float m_ttree_psi2 { 0.0 };
  float m_ttree_psi3 { 0.0 };

  int k_neta_cemc { -1 };
  int k_nphi_cemc { -1 };
  int k_neta_hcalin { -1 };
  int k_nphi_hcalin { -1 };
  int k_neta_hcalout { -1 };
  int k_nphi_hcalout { -1 };
  std::vector< std::vector< float > > * m_cemc_tower_energy {};
  std::vector< std::vector< int > >   * m_cemc_tower_status {};
  std::vector< std::vector< float > > * m_hcalin_tower_energy {};
  std::vector< std::vector< int > >  * m_hcalin_tower_status {};
  std::vector< std::vector< float > > * m_hcalout_tower_energy {};
  std::vector< std::vector< int > > * m_hcalout_tower_status {};

  float m_b { 0.0 };
  float m_ep_angle { 0.0 };
  float m_ecc { 0.0 };
  float m_ncoll { 0.0 };
  float m_npart { 0.0 };
  float m_psi1 { 0.0 };
  float m_psi2 { 0.0 };
  float m_psi3 { 0.0 };
  int m_runnumber { -1 };

  float m_scale_factor { 1.0 };
  
 
  
};

#endif  // CaloManip_CaloManip_H
