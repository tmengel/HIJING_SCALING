#include "CaloManip.h"

#include <fun4all/PHTFileServer.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>

#include <phool/getClass.h>

#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>

#include <ffaobjects/EventHeaderv1.h>

#include <TFile.h>
#include <TTree.h>

int CaloManip::Init( PHCompositeNode * /*topNode*/ )
{
  
  m_tfile = TFile::Open( m_input_file.c_str(), "READ" );
  if ( !m_tfile || m_tfile->IsZombie() ) 
  {
    std::cout << "CaloManip::Init - Fatal Error - cannot open input tree file " << m_input_file << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  m_ttree = dynamic_cast<TTree*>( m_tfile->Get("T") );
  if ( !m_ttree ) 
  {
    std::cout << "CaloManip::Init - Fatal Error - cannot find tree T in input file " << m_input_file << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  k_neta_hcalin = 24;
  k_nphi_hcalin = 64;
  k_neta_hcalout = 24;
  k_nphi_hcalout = 64;
  if ( std::string::npos != m_cemc_node.find("RETOWER") )
  {
    k_neta_cemc = 24;
    k_nphi_cemc = 64;
  }
  else
  {
    k_neta_cemc = 96;
    k_nphi_cemc = 256;
  }

  std::cout << "CaloManip::Init - Tower geometry: CEMC " << k_neta_cemc << "x" << k_nphi_cemc
            << ", HCALIN " << k_neta_hcalin << "x" << k_nphi_hcalin
            << ", HCALOUT " << k_neta_hcalout << "x" << k_nphi_hcalout
            << std::endl;

  if (Verbosity() > 2 )
  {
    m_ttree -> Show(0);
  }

  m_ttree -> SetBranchStatus( "*", false );
  m_ttree -> SetBranchStatus( "runnumber", true );
  m_ttree -> SetBranchStatus( "b", true );
  m_ttree -> SetBranchStatus( "ep_angle", true );
  m_ttree -> SetBranchStatus( "ecc", true );
  m_ttree -> SetBranchStatus( "ncoll", true );
  m_ttree -> SetBranchStatus( "psi1", true );
  m_ttree -> SetBranchStatus( "psi2", true );
  m_ttree -> SetBranchStatus( "psi3", true );
  m_ttree -> SetBranchStatus( "cemc_tower_energy", true );
  m_ttree -> SetBranchStatus( "cemc_tower_status", true );
  m_ttree -> SetBranchStatus( "hcalin_tower_energy", true );
  m_ttree -> SetBranchStatus( "hcalin_tower_status", true );
  m_ttree -> SetBranchStatus( "hcalout_tower_energy", true );
  m_ttree -> SetBranchStatus( "hcalout_tower_status", true );
  m_ttree -> SetBranchAddress( "runnumber", &m_ttree_runnumber );
  m_ttree -> SetBranchAddress( "b", &m_ttree_b );
  m_ttree -> SetBranchAddress( "ep_angle", &m_ttree_ep_angle );
  m_ttree -> SetBranchAddress( "ecc", &m_ttree_ecc );
  m_ttree -> SetBranchAddress( "ncoll", &m_ttree_ncoll );
  m_ttree -> SetBranchAddress( "psi1", &m_ttree_psi1 );
  m_ttree -> SetBranchAddress( "psi2", &m_ttree_psi2 );
  m_ttree -> SetBranchAddress( "psi3", &m_ttree_psi3 );
  m_ttree -> SetBranchAddress( "cemc_tower_energy", &m_cemc_tower_energy );
  m_ttree -> SetBranchAddress( "cemc_tower_status", &m_cemc_tower_status );
  m_ttree -> SetBranchAddress( "hcalin_tower_energy", &m_hcalin_tower_energy );
  m_ttree -> SetBranchAddress( "hcalin_tower_status", &m_hcalin_tower_status );
  m_ttree -> SetBranchAddress( "hcalout_tower_energy", &m_hcalout_tower_energy );
  m_ttree -> SetBranchAddress( "hcalout_tower_status", &m_hcalout_tower_status );

  m_event_id = -1;
  m_nentries_in_tree =  m_ttree -> GetEntries();
  if ( Verbosity() > 0 )
  {
    std::cout << "CaloManip: opened input tree file " << m_input_file << " with " << m_nentries_in_tree << " entries" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;

}

int CaloManip::process_event( PHCompositeNode * topNode )
{

  m_event_id++; 

  if( Verbosity() > 1 ) 
  {
    std::cout << "CaloManip::process_event - Process event " << m_event_id << std::endl;
  }

  auto * eventhead = findNode::getClass<EventHeader>( topNode, m_eventhead_node );
  if ( !eventhead )
  {
    std::cout << PHWHERE << " Input node " << m_eventhead_node << " Node missing, doing nothing." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  m_runnumber = eventhead->get_RunNumber();
  m_b = eventhead->get_ImpactParameter();
  m_ep_angle = eventhead->get_EventPlaneAngle();
  m_ecc = eventhead->get_eccentricity();
  m_ncoll = eventhead->get_ncoll();
  m_npart = eventhead->get_npart();
  m_psi1 = eventhead->get_FlowPsiN(1);
  m_psi2 = eventhead->get_FlowPsiN(2);
  m_psi3 = eventhead->get_FlowPsiN(3);
  if ( Verbosity() > 1 ) 
  {
    std::cout << PHWHERE << " - runnumber = " << m_runnumber << ", b = " << m_b << ", ep_angle = " << m_ep_angle << ", ecc = " << m_ecc << ", psi2 = " << m_psi2 << ", ncoll = " << m_ncoll << ", npart = " << m_npart << std::endl;
  }

  auto * cemc_towers = findNode::getClass<TowerInfoContainer>( topNode, m_cemc_node );
  if ( !cemc_towers )
  {
    std::cout << PHWHERE << " Input node " << m_cemc_node << " Node missing, doing nothing." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  auto * hcalin_towers = findNode::getClass<TowerInfoContainer>( topNode, m_hcalin_node );
  if ( !hcalin_towers )
  {
    std::cout << PHWHERE << " Input node " << m_hcalin_node << " Node missing, doing nothing." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  auto * hcalout_towers = findNode::getClass<TowerInfoContainer>( topNode, m_hcalout_node );
  if ( !hcalout_towers )
  {
    std::cout << PHWHERE << " Input node " << m_hcalout_node << " Node missing, doing nothing." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  if ( m_event_id >= m_nentries_in_tree )
  {
    std::cout << PHWHERE << " Event ID " << m_event_id << " exceeds number of entries in tree " << m_nentries_in_tree << ", skipping. " << std::endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  m_ttree -> GetEntry( m_event_id );
  if ( m_runnumber != m_ttree_runnumber 
    || m_b != m_ttree_b
    || m_ep_angle != m_ttree_ep_angle
    || m_ecc != m_ttree_ecc
    || m_ncoll != m_ttree_ncoll
    || m_psi1 != m_ttree_psi1
    || m_psi2 != m_ttree_psi2
    || m_psi3 != m_ttree_psi3 
  )
  {
    std::cout << PHWHERE << " Event ID " << m_event_id << " does not match tree event data, skipping. " << std::endl;
    std::cout << "  runnumber: " << m_runnumber << " vs " << m_ttree_runnumber << std::endl;
    std::cout << "  b: " << m_b << " vs " << m_ttree_b << std::endl;
    std::cout << "  ep_angle: " << m_ep_angle << " vs " << m_ttree_ep_angle << std::endl;
    std::cout << "  ecc: " << m_ecc << " vs " << m_ttree_ecc << std::endl;
    std::cout << "  ncoll: " << m_ncoll << " vs " << m_ttree_ncoll << std::endl;
    std::cout << "  psi1: " << m_psi1 << " vs " << m_ttree_psi1 << std::endl;
    std::cout << "  psi2: " << m_psi2 << " vs " << m_ttree_psi2 << std::endl;
    std::cout << "  psi3: " << m_psi3 << " vs " << m_ttree_psi3 << std::endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

 
  // cemc
  int ntowers = cemc_towers -> size();
  for ( auto ich = 0; ich < ntowers; ++ich )
  {
    auto * tower   = cemc_towers -> get_tower_at_channel( ich );
    if ( !tower )
    { 
      continue;
    }

    const auto key = cemc_towers -> encode_key( ich );    
    auto ieta      = cemc_towers -> getTowerEtaBin( key );
    auto iphi      = cemc_towers -> getTowerPhiBin( key );
    
    float tree_energy = m_cemc_tower_energy->at(ieta).at(iphi);
    int tree_isgood = m_cemc_tower_status->at(ieta).at(iphi);
    if ( tree_isgood != 1  || !tower->get_isGood() )
    {
      continue;
    }

    float this_E = tower -> get_energy(); // embedded
    float new_E = this_E +  (tree_energy * ( m_scale_factor - 1.0 )); // apply energy scale to tree energy (UE) and add to embedded
    if ( Verbosity() > 3 )
    {
      std::cout << PHWHERE << " CEMC Tower channel " << ich 
        << " new energy: " << new_E 
        << " ( original energy: " << tower -> get_energy() << " ) " 
        << std::endl;
    }
  }

  ntowers = hcalin_towers -> size();
  for ( auto ich = 0; ich < ntowers; ++ich )
  {
    auto * tower   = hcalin_towers -> get_tower_at_channel( ich );
    if ( !tower )
    { 
      continue;
    }

    const auto key = hcalin_towers -> encode_key( ich );    
    auto ieta      = hcalin_towers -> getTowerEtaBin( key );
    auto iphi      = hcalin_towers -> getTowerPhiBin( key );
    
    float tree_energy = m_hcalin_tower_energy->at(ieta).at(iphi);
    int tree_isgood = m_hcalin_tower_status->at(ieta).at(iphi);
    if ( tree_isgood != 1  || !tower->get_isGood() )
    {
      continue;
    }

    float this_E = tower -> get_energy(); // embedded
    float new_E = this_E +  (tree_energy * ( m_scale_factor - 1.0 )); // apply energy scale to tree energy (UE) and add to embedded
    if ( Verbosity() > 3 )
    {
      std::cout << PHWHERE << " HCALIN Tower channel " << ich 
        << " new energy: " << new_E 
        << " ( original energy: " << tower -> get_energy() << " ) " 
        << std::endl;
    }
  }

  ntowers = hcalout_towers -> size();
  for ( auto ich = 0; ich < ntowers; ++ich )
  {
    auto * tower   = hcalout_towers -> get_tower_at_channel( ich );
    if ( !tower )
    { 
      continue;
    }

    const auto key = hcalout_towers -> encode_key( ich );    
    auto ieta      = hcalout_towers -> getTowerEtaBin( key );
    auto iphi      = hcalout_towers -> getTowerPhiBin( key );

    float tree_energy = m_hcalout_tower_energy->at(ieta).at(iphi);
    int tree_isgood = m_hcalout_tower_status->at(ieta).at(iphi);
    if ( tree_isgood != 1  || !tower->get_isGood() )
    {
      continue;
    }

    float this_E = tower -> get_energy(); // embedded
    float new_E = this_E +  (tree_energy * ( m_scale_factor - 1.0 )); // apply energy scale to tree energy (UE) and add to embedded
    if ( Verbosity() > 3 )
    {
      std::cout << PHWHERE << " HCALOUT Tower channel " << ich 
        << " new energy: " << new_E 
        << " ( original energy: " << tower -> get_energy() << " ) "
        << std::endl;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CaloManip::End( PHCompositeNode * /*topNode*/ )
{
  
  m_tfile -> Close();
  if ( Verbosity () > 0 ) 
  {
    std::cout << "CaloManip::End - done" << std::endl;
  }
  return Fun4AllReturnCodes::EVENT_OK;

}