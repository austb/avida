/*
 *  cStats.cc
 *  Avida
 *
 *  Called "stats.cc" prior to 12/5/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1993-2001 California Institute of Technology.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cStats.h"

#include "avida/core/WorldDriver.h"
#include "avida/data/Manager.h"
#include "avida/data/Package.h"

#include "cBioGroup.h"
#include "cDataFile.h"
#include "cEnvironment.h"
#include "cHardwareBase.h"
#include "cHardwareManager.h"
#include "cInstSet.h"
#include "cPopulation.h"
#include "cPopulationCell.h"
#include "cDeme.h"
#include "cMigrationMatrix.h" // MIGRATION_MATRIX
#include "cStringUtil.h"
#include "cWorld.h"
#include "tDataEntry.h"
#include "cOrgMessage.h"
#include "cOrgMessagePredicate.h"
#include "cOrgMovementPredicate.h"
#include "cReaction.h"
#include "cEventList.h"

#include <algorithm>
#include <cfloat>
#include <numeric>
#include <cmath>
#include <sstream>

using namespace Avida;
using namespace AvidaTools;


cStats::cStats(cWorld* world)
  : m_world(world)
  , m_data_manager(this, "population_data")
  , m_update(-1)
  , avida_time(0)
  , rave_true_replication_rate( 500 )
  , entropy(0.0)
  , species_entropy(0.0)
  , energy(0.0)
  , dom_fidelity(0.0)
  , ave_fidelity(0.0)
  , max_viable_fitness(0)
  , dom_merit(0)
  , dom_gestation(0)
  , dom_repro_rate(0)
  , dom_fitness(0)
  , dom_size(0)
  , dom_copied_size(0)
  , dom_exe_size(0)
  , max_fitness(0)
  , max_merit(0)
  , max_gestation_time(0)
  , max_genome_length(0)
  , min_fitness(FLT_MAX)
  , min_merit(FLT_MAX)
  , min_gestation_time(INT_MAX)
  , min_genome_length(INT_MAX)
  , dom_genotype_id(-1)
  , dom_name("(none)")
  , dom_births(0)
  , dom_breed_true(0)
  , dom_breed_in(0)
  , dom_breed_out(0)
  , dom_abundance(0)
  , dom_gene_depth(-1)
  , dom_sequence("")
  , dom_last_birth_cell(0)
  , dom_last_forager_type(-1)
  , dom_last_group_id(-1)
  , coal_depth(0)
  , num_births(0)
  , num_deaths(0)
  , num_breed_in(0)
  , num_breed_true(0)
  , num_breed_true_creatures(0)
  , num_creatures(0)
  , num_genotypes(0)
  , num_genotypes_historic(0)
  , num_threshold(0)
  , num_lineages(0)
  , num_executed(0)
  , num_parasites(0)
  , num_no_birth_creatures(0)
  , num_single_thread_creatures(0)
  , num_multi_thread_creatures(0)
  , m_num_threads(0)
  , num_modified(0)
  , num_genotypes_last(1)
  , tot_organisms(0)
  , tot_genotypes(0)
  , tot_threshold(0)
  , tot_lineages(0)
  , tot_executed(0)
  , num_resamplings(0)
  , num_failedResamplings(0)
  , last_update(0)
  , num_bought(0)
  , num_sold(0)
  , num_used(0)
  , num_own_used(0)
  , sense_size(0)
  , avg_competition_fitness(0)
  , min_competition_fitness(0)
  , max_competition_fitness(0)
  , avg_competition_copied_fitness(0)
  , min_competition_copied_fitness(0)
  , max_competition_copied_fitness(0)
  , num_orgs_replicated(0)
  , m_spec_total(0)
  , m_spec_num(0)
  , m_spec_waste(0)
  , num_migrations(0)
  , m_num_successful_mates(0)
  , prey_entropy(0.0)
  , pred_entropy(0.0)
  , topreac(-1)
  , topcycle(-1)
  , m_deme_num_repls(0)
	, m_deme_num_repls_treatable(0)
	, m_deme_num_repls_untreatable(0)
  , m_donate_to_donor (0)
  , m_donate_to_facing (0)
{
  const cEnvironment& env = m_world->GetEnvironment();
  const int num_tasks = env.GetNumTasks();

  task_cur_count.Resize(num_tasks);
  task_last_count.Resize(num_tasks);

  tasks_host_current.Resize(num_tasks);
  tasks_host_last.Resize(num_tasks);
  tasks_parasite_current.Resize(num_tasks);
  tasks_parasite_last.Resize(num_tasks);

  task_cur_quality.Resize(num_tasks);
  task_last_quality.Resize(num_tasks);
  task_cur_max_quality.Resize(num_tasks);
  task_last_max_quality.Resize(num_tasks);
  task_exe_count.Resize(num_tasks);
  new_task_count.Resize(num_tasks);
  prev_task_count.Resize(num_tasks);
  cur_task_count.Resize(num_tasks);
  new_reaction_count.Resize(env.GetNumReactions());
  task_cur_count.SetAll(0);
  task_cur_quality.SetAll(0);
  task_cur_max_quality.SetAll(0);
  task_last_max_quality.SetAll(0);
  task_last_quality.SetAll(0);
  task_last_count.SetAll(0);
  task_cur_max_quality.SetAll(0);
  task_last_max_quality.SetAll(0);
  task_exe_count.SetAll(0);
  new_task_count.SetAll(0);
  prev_task_count.SetAll(0);
  cur_task_count.SetAll(0);
  new_reaction_count.SetAll(0);

  // Stats for internal resource use
  task_internal_cur_count.Resize(num_tasks);
  task_internal_last_count.Resize(num_tasks);
  task_internal_cur_quality.Resize(num_tasks);
  task_internal_last_quality.Resize(num_tasks);
  task_internal_cur_max_quality.Resize(num_tasks);
  task_internal_last_max_quality.Resize(num_tasks);
  task_internal_cur_count.SetAll(0);
  task_internal_last_count.SetAll(0);
  task_internal_cur_quality.SetAll(0.0);
  task_internal_last_quality.SetAll(0.0);
  task_internal_cur_max_quality.SetAll(0.0);
  task_internal_last_max_quality.SetAll(0.0);


  ZeroInst();
  ZeroFTInst();

  const int num_reactions = env.GetNumReactions();
  m_reaction_cur_count.Resize(num_reactions);
  m_reaction_last_count.Resize(num_reactions);
  m_reaction_cur_add_reward.Resize(num_reactions);
  m_reaction_last_add_reward.Resize(num_reactions);
  m_reaction_exe_count.Resize(num_reactions);
  m_reaction_cur_count.SetAll(0);
  m_reaction_last_count.SetAll(0);
  m_reaction_cur_add_reward.SetAll(0.0);
  m_reaction_last_add_reward.SetAll(0.0);
  m_reaction_exe_count.SetAll(0);


  resource_count.Resize( m_world->GetNumResources() );
  resource_count.SetAll(0);

  resource_geometry.Resize( m_world->GetNumResources() );
  resource_geometry.SetAll(nGeometry::GLOBAL);

  task_names.Resize(num_tasks);
  for (int i = 0; i < num_tasks; i++) task_names[i] = env.GetTask(i).GetDesc();

  reaction_names.Resize(num_reactions);
  for (int i = 0; i < num_reactions; i++) reaction_names[i] = env.GetReactionName(i);

  resource_names.Resize( m_world->GetNumResources() );
  
  // This block calculates how many slots we need to
  // make for paying attention to different label combinations
  // Require sense instruction to be present then die if not at least 2 NOPs

  // @DMB - This code makes assumptions about instruction sets that may not hold true under multiple inst sets.
  //      - This sort of functionality should be reimplemented as instruction set stats or something similar
//  bool sense_used = m_world->GetHardwareManager().GetInstSet().InstInSet( cStringUtil::Stringf("sense") )
//                ||  m_world->GetHardwareManager().GetInstSet().InstInSet( cStringUtil::Stringf("sense-unit") )
//                ||  m_world->GetHardwareManager().GetInstSet().InstInSet( cStringUtil::Stringf("sense-m100") );
//  if (sense_used)
//  {
//    if (m_world->GetHardwareManager().GetInstSet().GetNumNops() < 2)
//    {
//      cerr << "Error: If you have a sense instruction in your instruction set, then";
//      cerr << "you MUST also include at least two NOPs in your instruction set. " << endl; exit(1);
//    }
//
//    int on = 1;
//    int max_sense_label_length = 0;
//    while (on < m_world->GetNumResources())
//    {
//      max_sense_label_length++;
//      sense_size += on;
//      on *= m_world->GetHardwareManager().GetInstSet().GetNumNops();
//    }
//    sense_size += on;
//
//    sense_last_count.Resize( sense_size );
//    sense_last_count.SetAll(0);
//
//    sense_last_exe_count.Resize( sense_size );
//    sense_last_exe_count.SetAll(0);
//
//    sense_names.Resize( sense_size );
//    int assign_index = 0;
//    int num_per = 1;
//    for (int i=0; i<= max_sense_label_length; i++)
//    {
//      for (int j=0; j< num_per; j++)
//      {
//        sense_names[assign_index] = (on > 1) ?
//          cStringUtil::Stringf("sense_res.%i-%i", j*on, (j+1)*on-1) :
//          cStringUtil::Stringf("sense_res.%i", j);
//
//        assign_index++;
//      }
//      on /= m_world->GetHardwareManager().GetInstSet().GetNumNops();
//      num_per *= m_world->GetHardwareManager().GetInstSet().GetNumNops();
//    }
//  }
  // End sense tracking initialization

  if(m_world->GetConfig().NUM_DEMES.Get() == 0) {
    relative_pos_event_count.ResizeClear(m_world->GetConfig().WORLD_X.Get(), m_world->GetConfig().WORLD_Y.Get());
    relative_pos_pred_sat.ResizeClear(m_world->GetConfig().WORLD_X.Get(), m_world->GetConfig().WORLD_Y.Get());
  } else {
    relative_pos_event_count.ResizeClear(m_world->GetConfig().WORLD_X.Get(), m_world->GetConfig().WORLD_Y.Get() / m_world->GetConfig().NUM_DEMES.Get());
    relative_pos_pred_sat.ResizeClear(m_world->GetConfig().WORLD_X.Get(), m_world->GetConfig().WORLD_Y.Get() / m_world->GetConfig().NUM_DEMES.Get());
  }

  relative_pos_event_count.SetAll(0);
  relative_pos_pred_sat.SetAll(0);

  setupProvidedData();
}


void cStats::NotifyBGEvent(cBioGroup* bg, eBGEventType type, cBioUnit* bu)
{
  assert(bg);

  switch (type) {
    case BG_EVENT_ADD_THRESHOLD:
      num_threshold++;
      tot_threshold++;
      if (m_world->GetConfig().LOG_THRESHOLD.Get()) {
        cDataFile& df = m_world->GetDataFile("threshold.log");
        df.Write(m_update, "Update");
        df.Write(bg->GetID(), "ID");
        df.Write(bg->GetProperty("name").AsString(), "Name");
        df.Endl();
      }
      break;

    case BG_EVENT_REMOVE_THRESHOLD:
      num_threshold--;
      break;
  }

}


Data::ConstDataSetPtr cStats::Provides() const
{
  if (!m_provides) {
    Data::DataSetPtr provides(new Apto::Set<Apto::String>);
    for (Apto::Map<Apto::String, ProvidedData>::KeyIterator it = m_provided_data.Keys(); it.Next();) {
      provides->Insert(*it.Get());
    }
    m_provides = provides;
  }
  return m_provides;
}

void cStats::UpdateProvidedValues(Update current_update)
{
  // Nothing for now, all handled by ProcessUpdate()
}

Data::PackagePtr cStats::GetProvidedValue(const Apto::String& data_id) const
{
  ProvidedData data_entry;
  Data::PackagePtr rtn;
  if (m_provided_data.Get(data_id, data_entry)) {
    rtn = data_entry.GetData();
  }
  assert(rtn);
  return rtn;
}

Apto::String cStats::DescribeProvidedValue(const Apto::String& data_id) const
{
  ProvidedData data_entry;
  Apto::String rtn;
  if (m_provided_data.Get(data_id, data_entry)) {
    rtn = data_entry.description;
  }
  assert(rtn != "");
  return rtn;
}


template <class T> Data::PackagePtr cStats::packageData(T (cStats::*func)() const) const
{
  return Data::PackagePtr(new Data::Wrap<T>((this->*func)()));
}


void cStats::setupProvidedData()
{
  // Load in all the keywords, descriptions, and associated functions for
  // data management.
  
  // Setup functors and references for use in the PROVIDE macro
  Data::ProviderActivateFunctor activate(m_world, &cWorld::GetStatsProvider);
  Data::Manager& mgr = m_world->GetDataManager();
  Apto::Functor<Data::PackagePtr, Apto::TL::Create<int (cStats::*)() const> > intStat(this, &cStats::packageData<int>);
  Apto::Functor<Data::PackagePtr, Apto::TL::Create<double (cStats::*)() const> > doubleStat(this, &cStats::packageData<double>);

  // Define PROVIDE macro to simplify instantiating new provided data
#define PROVIDE(name, desc, type, func) { \
  m_provided_data[name] = ProvidedData(desc, Apto::BindFirst(type ## Stat, &cStats::func));\
  mgr.Register(name, activate); \
}
  
  // Time Stats
  m_data_manager.Add("update",      "Update",      &cStats::GetUpdate);
  m_data_manager.Add("generation",  "Generation",  &cStats::GetGeneration);

  PROVIDE("core.update",                   "Update",                               int,    GetUpdate);
  PROVIDE("core.world.ave_generation",     "Average Generation",                   double, GetGeneration);
  

  // Population Level Stats
  m_data_manager.Add("entropy",         "Genotype Entropy (Diversity)", &cStats::GetEntropy);
  m_data_manager.Add("species_entropy", "Species Entropy (Diversity)",  &cStats::GetEntropy);
  m_data_manager.Add("energy",          "Average Inferiority (Energy)", &cStats::GetEnergy);
  m_data_manager.Add("richness",        "Number of Different Genotypes (Richness)", &cStats::GetNumGenotypes);
  m_data_manager.Add("eveness",         "Equitability of Genotype Distribution (Evenness)", &cStats::GetEvenness);
  m_data_manager.Add("coal_depth",      "Depth of Coalescent Genotype", &cStats::GetCoalescentDepth);
  m_data_manager.Add("num_resamplings",  "Total Number of resamplings this time step", &cStats::GetResamplings);
  m_data_manager.Add("num_failedResamplings",  "Total Number of divide commands that reached the resampling hard-cap this time step", &cStats::GetFailedResamplings);


  // Dominant Genotype Stats
  m_data_manager.Add("dom_merit",      "Ave Merit of Dominant Genotype",          &cStats::GetDomMerit);
  m_data_manager.Add("dom_gest",       "Ave Gestation Time of Dominant Genotype", &cStats::GetDomGestation);
  m_data_manager.Add("dom_fitness",    "Ave Fitness of Dominant Genotype",        &cStats::GetDomFitness);
  m_data_manager.Add("dom_repro",      "Ave Repro-Rate of Dominant Genotype",     &cStats::GetDomReproRate);
  m_data_manager.Add("dom_length",     "Genome Length of Dominant Genotype",      &cStats::GetDomSize);
  m_data_manager.Add("dom_copy_length","Copied Length of Dominant Genotype",      &cStats::GetDomCopySize);
  m_data_manager.Add("dom_exe_length", "Executed Length of Dominant Genotype",    &cStats::GetDomExeSize);
  m_data_manager.Add("dom_id",         "ID of Dominant Genotype",                 &cStats::GetDomID);
  m_data_manager.Add("dom_name",       "Name of Dominant Genotype",               &cStats::GetDomName);
  m_data_manager.Add("dom_births",     "Birth Count of Dominant Genotype",        &cStats::GetDomBirths);
  m_data_manager.Add("dom_breed_true", "Breed-True Count  of Dominant Genotype",  &cStats::GetDomBreedTrue);
  m_data_manager.Add("dom_breed_in",   "Breed-In Count of Dominant Genotype",     &cStats::GetDomBreedIn);
  m_data_manager.Add("dom_breed_out",  "Breed-Out Count of Dominant Genotype",    &cStats::GetDomBreedOut);
  m_data_manager.Add("dom_num_cpus",   "Abundance of Dominant Genotype",          &cStats::GetDomAbundance);
  m_data_manager.Add("dom_depth",      "Tree Depth of Dominant Genotype",         &cStats::GetDomGeneDepth);
  m_data_manager.Add("dom_sequence",   "Sequence of Dominant Genotype",           &cStats::GetDomSequence);
  m_data_manager.Add("dom_last_birth_cell", "Birth Cell of Last-Born Dominant Genotype", &cStats::GetDomLastBirthCell);
  m_data_manager.Add("dom_last_group_id", "Birth Group ID of Last-Born Dominant Genotype", &cStats::GetDomLastGroup);
  m_data_manager.Add("dom_last_forager_type", "Birth Forager Type of Last-Born Dominant Genotype", &cStats::GetDomLastForagerType);

  // Current Counts...
  m_data_manager.Add("num_births",     "Count of Births in Population",          &cStats::GetNumBirths);
  m_data_manager.Add("num_deaths",     "Count of Deaths in Population",          &cStats::GetNumDeaths);
  m_data_manager.Add("breed_in",       "Count of Non-Breed-True Births",         &cStats::GetBreedIn);
  m_data_manager.Add("breed_true",     "Count of Breed-True Births",             &cStats::GetBreedTrue);
  m_data_manager.Add("bred_true",      "Count of Organisms that have Bred True", &cStats::GetBreedTrueCreatures);
  m_data_manager.Add("num_cpus",       "Count of Organisms in Population",       &cStats::GetNumCreatures);
  m_data_manager.Add("num_genotypes",  "Count of Genotypes in Population",       &cStats::GetNumGenotypes);
  m_data_manager.Add("num_genotypes_historic", "Count of Historic Genotypes",    &cStats::GetNumGenotypesHistoric);
  m_data_manager.Add("num_threshold",  "Count of Threshold Genotypes",           &cStats::GetNumThreshold);
  m_data_manager.Add("num_lineages",   "Count of Lineages in Population",        &cStats::GetNumLineages);
  m_data_manager.Add("num_parasites",  "Count of Parasites in Population",       &cStats::GetNumParasites);
  m_data_manager.Add("threads",        "Count of Threads in Population",         &cStats::GetNumThreads);
  m_data_manager.Add("num_no_birth",   "Count of Childless Organisms",           &cStats::GetNumNoBirthCreatures);

  PROVIDE("core.world.organisms",          "Count of Organisms in the World",      int,    GetNumCreatures);

  
  // Total Counts...
  m_data_manager.Add("tot_cpus",      "Total Organisms ever in Population", &cStats::GetTotCreatures);
  m_data_manager.Add("tot_genotypes", "Total Genotypes ever in Population", &cStats::GetTotGenotypes);
  m_data_manager.Add("tot_threshold", "Total Threshold Genotypes Ever",     &cStats::GetTotThreshold);
  m_data_manager.Add("tot_lineages",  "Total Lineages ever in Population",  &cStats::GetTotLineages);

  
  // Some Average Data...
  m_data_manager.Add("ave_repro_rate", "Average Repro-Rate (1/Gestation)", &cStats::GetAveReproRate);
  m_data_manager.Add("ave_merit",      "Average Merit",                    &cStats::GetAveMerit);
  m_data_manager.Add("ave_age",        "Average Age",                      &cStats::GetAveCreatureAge);
  m_data_manager.Add("ave_memory",     "Average Memory Used",              &cStats::GetAveMemSize);
  m_data_manager.Add("ave_neutral",    "Average Neutral Metric",           &cStats::GetAveNeutralMetric);
  m_data_manager.Add("ave_lineage",    "Average Lineage Label",            &cStats::GetAveLineageLabel);
  m_data_manager.Add("ave_gest",       "Average Gestation Time",           &cStats::GetAveGestation);
  m_data_manager.Add("ave_fitness",    "Average Fitness",                  &cStats::GetAveFitness);
  m_data_manager.Add("ave_gen_age",    "Average Genotype Age",             &cStats::GetAveGenotypeAge);
  m_data_manager.Add("ave_length",     "Average Genome Length",            &cStats::GetAveSize);
  m_data_manager.Add("ave_copy_length","Average Copied Length",            &cStats::GetAveCopySize);
  m_data_manager.Add("ave_exe_length", "Average Executed Length",          &cStats::GetAveExeSize);
  m_data_manager.Add("ave_thresh_age", "Average Threshold Genotype Age",   &cStats::GetAveThresholdAge);

  PROVIDE("core.world.ave_metabolic_rate", "Average Metabolic Rate",               double, GetAveMerit);
  PROVIDE("core.world.ave_age",            "Average Organism Age (in updates)",    double, GetAveCreatureAge);
  PROVIDE("core.world.ave_gestation_time", "Average Gestation Time",               double, GetAveGestation);
  PROVIDE("core.world.ave_fitness",        "Average Fitness",                      double, GetAveFitness);

  
  // Maximums
  m_data_manager.Add("max_fitness", "Maximum Fitness in Population", &cStats::GetMaxFitness);
  m_data_manager.Add("max_merit",   "Maximum Merit in Population",   &cStats::GetMaxMerit);

  
  // Minimums
  m_data_manager.Add("min_fitness", "Minimum Fitness in Population", &cStats::GetMinFitness);
  
#undef PROVIDE
}

void cStats::ZeroTasks()
{
  task_cur_count.SetAll(0);
  task_last_count.SetAll(0);

  tasks_host_current.SetAll(0);
  tasks_host_last.SetAll(0);
  tasks_parasite_current.SetAll(0);
  tasks_parasite_last.SetAll(0);

  task_cur_quality.SetAll(0);
  task_last_quality.SetAll(0);
  task_last_max_quality.SetAll(0);
  task_cur_max_quality.SetAll(0);
  task_internal_cur_count.SetAll(0);
  task_internal_cur_quality.SetAll(0);
  task_internal_cur_max_quality.SetAll(0);
  task_internal_last_count.SetAll(0);
  task_internal_last_quality.SetAll(0);
  task_internal_last_max_quality.SetAll(0);
}

void cStats::ZeroReactions()
{
  m_reaction_cur_count.SetAll(0);
  m_reaction_last_count.SetAll(0);
  m_reaction_cur_add_reward.SetAll(0);
  m_reaction_last_add_reward.SetAll(0);
}


void cStats::ZeroInst()
{
  for (tArrayMap<cString, tArray<cIntSum> >::iterator it = m_is_exe_inst_map.begin(); it != m_is_exe_inst_map.end(); it++) {
    for (int i = 0; i < (*it).Value().GetSize(); i++) (*it).Value()[i].Clear();
  }
}

void cStats::ZeroFTInst()
{
  for (tArrayMap<cString, tArray<cIntSum> >::iterator it = m_is_prey_exe_inst_map.begin(); it != m_is_prey_exe_inst_map.end(); it++) {
    for (int i = 0; i < (*it).Value().GetSize(); i++) (*it).Value()[i].Clear();
  }
  for (tArrayMap<cString, tArray<cIntSum> >::iterator it = m_is_pred_exe_inst_map.begin(); it != m_is_pred_exe_inst_map.end(); it++) {
    for (int i = 0; i < (*it).Value().GetSize(); i++) (*it).Value()[i].Clear();
  }
}

void cStats::ZeroMTInst()
{
  for (tArrayMap<cString, tArray<cIntSum> >::iterator it = m_is_male_exe_inst_map.begin(); it != m_is_male_exe_inst_map.end(); it++) {
    for (int i = 0; i < (*it).Value().GetSize(); i++) (*it).Value()[i].Clear();
  }
  for (tArrayMap<cString, tArray<cIntSum> >::iterator it = m_is_female_exe_inst_map.begin(); it != m_is_female_exe_inst_map.end(); it++) {
    for (int i = 0; i < (*it).Value().GetSize(); i++) (*it).Value()[i].Clear();
  }
}

void cStats::CalcEnergy()
{
  assert(sum_fitness.Average() >= 0.0);
  assert(dom_fitness >= 0);


  // Note: When average fitness and dominant fitness are close in value (i.e. should be identical)
  //       floating point rounding error can cause output variances.  To mitigate this, threshold
  //       caps off values that differ by less than it, flushing the effective output value to zero.
  const double ave_fitness = sum_fitness.Average();
  const double threshold = 1.0e-14;
  if (ave_fitness == 0.0 || dom_fitness == 0.0 || fabs(ave_fitness - dom_fitness) < threshold) {
    energy = 0.0;
  } else  {
    energy = Log(dom_fitness / ave_fitness);
  }
}

void cStats::CalcFidelity()
{
  // There is a (small) probability that when a random instruction is picked
  // after a mutation occurs, that it will be the original instruction again;
  // This needs to be adjusted for!

  double ave_num_insts = 0.0;
  for (tArrayMap<cString, tArray<cString> >::iterator it = m_is_inst_names_map.begin(); it != m_is_inst_names_map.end(); it++) {
    ave_num_insts += (*it).Value().GetSize();
  }
  ave_num_insts /= m_is_inst_names_map.GetSize();

  double adj = (ave_num_insts - 1.0) / ave_num_insts;

  double base_fidelity = (1.0 - adj * m_world->GetConfig().DIVIDE_MUT_PROB.Get()) *
    (1.0 - m_world->GetConfig().DIVIDE_INS_PROB.Get()) * (1.0 - m_world->GetConfig().DIVIDE_DEL_PROB.Get());

  double true_cm_rate = adj * m_world->GetConfig().COPY_MUT_PROB.Get();
  ave_fidelity = base_fidelity * pow(1.0 - true_cm_rate, sum_size.Average());
  dom_fidelity = base_fidelity * pow(1.0 - true_cm_rate, dom_size);
}

void cStats::RecordBirth(bool breed_true)
{
	if (m_world->GetEventsList()->CheckBirthInterruptQueue(tot_organisms) == true)
		m_world->GetEventsList()->ProcessInterrupt(m_world->GetDefaultContext());

  tot_organisms++;
  num_births++;

  if (breed_true) num_breed_true++;
  else num_breed_in++;
}

void cStats::RemoveGenotype(int id_num, int parent_id,
   int parent_dist, int depth, int max_abundance, int parasite_abundance,
   int age, int length)
{
  if (m_world->GetConfig().LOG_GENOTYPES.Get() &&
      (m_world->GetConfig().LOG_GENOTYPES.Get() != 2 || max_abundance > 2)) {
    const int update_born = cStats::GetUpdate() - age + 1;
    cDataFile& df = m_world->GetDataFile("genotype.log");
    df.Write(id_num, "Genotype ID");
    df.Write(update_born, "Update Born");
    df.Write(parent_id, "Parent ID");
    df.Write(parent_dist, "Parent Distance");
    df.Write(depth, "Depth");
    df.Write(max_abundance, "Maximum Abundance");
    df.Write(age, "Age");
    df.Write(length, "Length");
    df.Endl();
  }

  (void) parasite_abundance; // Not used now, but maybe in future.
}

void cStats::ProcessUpdate()
{
  // Increment the "avida_time"
  if (sum_merit.Count() > 0 && sum_merit.Average() > 0) {
    double delta = ((double)(m_update-last_update))/sum_merit.Average();
    avida_time += delta;

    // calculate the true replication rate in this update
    rave_true_replication_rate.Add( num_births/
	  (delta * m_world->GetConfig().AVE_TIME_SLICE.Get() * num_creatures) );
  }
  last_update = m_update;

  // Zero-out any variables which need to be cleared at end of update.

  num_births = 0;
  num_deaths = 0;
  num_breed_true = 0;

  tot_executed += num_executed;
  num_executed = 0;

  task_cur_count.SetAll(0);
  task_last_count.SetAll(0);
  task_cur_quality.SetAll(0);
  task_last_quality.SetAll(0);
  task_cur_max_quality.SetAll(0);
  task_last_max_quality.SetAll(0);
  task_exe_count.SetAll(0);

  task_internal_cur_count.SetAll(0);
  task_internal_last_count.SetAll(0);
  task_internal_cur_quality.SetAll(0);
  task_internal_last_quality.SetAll(0);
  task_internal_cur_max_quality.SetAll(0);
  task_internal_last_max_quality.SetAll(0);

  sense_last_count.SetAll(0);
  sense_last_exe_count.SetAll(0);

  m_reaction_cur_count.SetAll(0);
  m_reaction_last_count.SetAll(0);
  m_reaction_cur_add_reward.SetAll(0.0);
  m_reaction_last_add_reward.SetAll(0.0);
  m_reaction_exe_count.SetAll(0);

  dom_merit = 0;
  dom_gestation = 0.0;
  dom_fitness = 0.0;
  max_fitness = 0.0;

  num_resamplings = 0;
  num_failedResamplings = 0;

  m_spec_total = 0;
  m_spec_num = 0;
  m_spec_waste = 0;

  num_migrations = 0;
  
  m_num_successful_mates = 0;
}

void cStats::RemoveLineage(int id_num, int parent_id, int update_born, double generation_born, int total_CPUs,
                           int total_genotypes, double fitness, double lineage_stat1, double lineage_stat2 )
{
  num_lineages--;
  if (m_world->GetConfig().LOG_LINEAGES.Get()) {
    cDataFile& lineage_log = m_world->GetDataFile("lineage.log");

    lineage_log.WriteComment("Columns 10, 11 depend on lineage creation method chosen.");

    lineage_log.Write(id_num, "lineage id");
    lineage_log.Write(parent_id, "parent lineage id");
    lineage_log.Write(fitness, "initial fitness");
    lineage_log.Write(total_CPUs, "total number of creatures");
    lineage_log.Write(total_genotypes, "total number of genotypes");
    lineage_log.Write(update_born, "update born");
    lineage_log.Write(cStats::GetUpdate(), "update extinct");
    lineage_log.Write(generation_born, "generation born");
    lineage_log.Write(SumGeneration().Average(), "generation extinct");
    lineage_log.Write(lineage_stat1, "lineage stat1");
    lineage_log.Write(lineage_stat2, "lineage stat2");
    lineage_log.Endl();
  }
}

int cStats::GetNumPreyCreatures() const
{ 
  return m_world->GetPopulation().GetNumPreyOrganisms(); 
}

int cStats::GetNumPredCreatures() const
{ 
  return m_world->GetPopulation().GetNumPredOrganisms(); 
}

void cStats::PrintDataFile(const cString& filename, const cString& format, char sep)
{
  cDataFile& data_file = m_world->GetDataFile(filename);
  m_data_manager.PrintRow(data_file, format, sep);
}


void cStats::PrintAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Average Data");
  df.WriteTimeStamp();

  df.Write(m_update,                "Update");
  df.Write(sum_merit.Average(),           "Merit");
  df.Write(sum_gestation.Average(),       "Gestation Time");
  df.Write(sum_fitness.Average(),         "Fitness");
  df.Write(sum_repro_rate.Average(),      "Repro Rate?");
  df.Write(sum_size.Average(),            "Size");
  df.Write(sum_copy_size.Average(),       "Copied Size");
  df.Write(sum_exe_size.Average(),        "Executed Size");
  df.Write(sum_abundance.Average(),       "Abundance");

  // The following causes births and breed true to default to 0.0 when num_creatures is 0
  double ave_births = 0.0;
  double ave_breed_true = 0.0;
  if (num_creatures > 0) {
    const double d_num_creatures = static_cast<double>(num_creatures);
    ave_births = static_cast<double>(num_births) / d_num_creatures;
    ave_breed_true = static_cast<double>(num_breed_true) / d_num_creatures;
  }
  df.Write(ave_births,                    "Proportion of organisms that gave birth in this update");
  df.Write(ave_breed_true,                "Proportion of Breed True Organisms");

  df.Write(sum_genotype_depth.Average(),  "Genotype Depth");
  df.Write(sum_generation.Average(),      "Generation");
  df.Write(sum_neutral_metric.Average(),  "Neutral Metric");
  df.Write(sum_lineage_label.Average(),   "Lineage Label");
  df.Write(rave_true_replication_rate.Average(), "True Replication Rate (based on births/update, time-averaged)");
  df.Endl();
}

void cStats::PrintDemeAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Average Deme Data");
  df.WriteTimeStamp();

  df.Write(m_update,                                        "Update");
  df.Write(m_num_occupied_demes,                            "Count");
  df.Write(sum_deme_age.Average(),                          "Age");
  df.Write(sum_deme_birth_count.Average(),                  "Births");
  df.Write(sum_deme_org_count.Average(),                    "Organisms");
  df.Write(sum_deme_generation.Average(),                   "Generation");
  df.Write(sum_deme_last_birth_count.Average(),                  "Births (at last replication)");
  df.Write(sum_deme_last_org_count.Average(),                    "Organisms (at last replication)");
  df.Write(sum_deme_merit.Average(),                        "Merit");
  df.Write(sum_deme_gestation_time.Average(),               "Gestation Time");
  df.Write(sum_deme_normalized_time_used.Average(),         "Time Used (normalized by org fitness)");
  df.Write(sum_deme_generations_per_lifetime.Average(),     "Generations between current and last founders");
  df.Write(sum_deme_events_killed.Average(),                "Events killed");
  df.Write(sum_deme_events_kill_attempts.Average(),         "Attempts to kill event");

  df.Endl();
}

void cStats::PrintFlowRateTuples(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Flow Rate Tuples");
  df.WriteTimeStamp();

  df.Write(m_update,                                        "Update");
  // write each tuple
  for(map<int, flow_rate_tuple>::iterator iter = flow_rate_tuples.begin(); iter != flow_rate_tuples.end(); iter++) {
    ostringstream oss;
    oss << "flow rate " << (*iter).first;
    string flow_rate_str(oss.str());
    string flow_rate_pop_size_str(flow_rate_str+" deme pop size");
    string flow_rate_events_killed_str(flow_rate_str+" events killed");
    string flow_rate_events_attempted_to_kill_str(flow_rate_str+" events attempted to kill");
    string flow_rate_exe_ratio_str(flow_rate_str+" exe ratio");
    string flow_rate_total_births_str(flow_rate_str+" total births");
    string flow_rate_total_sleeping_str(flow_rate_str+" total sleeping");

    df.Write((*iter).first, flow_rate_str.c_str());
    df.Write((*iter).second.orgCount.Average(), flow_rate_pop_size_str.c_str());
    df.Write((*iter).second.eventsKilled.Average(), flow_rate_events_killed_str.c_str());
    df.Write((*iter).second.attemptsToKillEvents.Average(), flow_rate_events_attempted_to_kill_str.c_str());
    df.Write((*iter).second.AvgEnergyUsageRatio.Average(), flow_rate_exe_ratio_str.c_str());
    df.Write((*iter).second.totalBirths.Average(), flow_rate_total_births_str.c_str());
    df.Write((*iter).second.currentSleeping.Average(), flow_rate_total_sleeping_str.c_str());

  }
  df.Endl();

  // reset all tuples
  for(map<int, flow_rate_tuple >::iterator iter = flow_rate_tuples.begin(); iter != flow_rate_tuples.end(); iter++) {
    (*iter).second.orgCount.Clear();
    (*iter).second.eventsKilled.Clear();
    (*iter).second.attemptsToKillEvents.Clear();
    (*iter).second.AvgEnergyUsageRatio.Clear();
    (*iter).second.totalBirths.Clear();
    (*iter).second.currentSleeping.Clear();
  }
}

void cStats::PrintErrorData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Standard Error Data");
  df.WriteTimeStamp();

  df.Write(m_update,                 "Update");
  df.Write(sum_merit.StdError(),           "Merit");
  df.Write(sum_gestation.StdError(),       "Gestation Time");
  df.Write(sum_fitness.StdError(),         "Fitness");
  df.Write(sum_repro_rate.StdError(),      "Repro Rate?");
  df.Write(sum_size.StdError(),            "Size");
  df.Write(sum_copy_size.StdError(),       "Copied Size");
  df.Write(sum_exe_size.StdError(),        "Executed Size");
  df.Write(sum_abundance.StdError(),       "Abundance");
  df.Write(-1,                             "(No Data)");
  df.Write(-1,                             "(No Data)");
  df.Write(sum_genotype_depth.StdError(),  "Genotype Depth");
  df.Write(sum_generation.StdError(),      "Generation");
  df.Write(sum_neutral_metric.StdError(),  "Neutral Metric");
  df.Write(sum_lineage_label.StdError(),   "Lineage Label");
  df.Write(rave_true_replication_rate.StdError(), "True Replication Rate (based on births/update, time-averaged)");
  df.Endl();
}


void cStats::PrintVarianceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Variance Data");
  df.WriteTimeStamp();

  df.Write(m_update,                 "Update");
  df.Write(sum_merit.Variance(),           "Merit");
  df.Write(sum_gestation.Variance(),       "Gestation Time");
  df.Write(sum_fitness.Variance(),         "Fitness");
  df.Write(sum_repro_rate.Variance(),      "Repro Rate?");
  df.Write(sum_size.Variance(),            "Size");
  df.Write(sum_copy_size.Variance(),       "Copied Size");
  df.Write(sum_exe_size.Variance(),        "Executed Size");
  df.Write(sum_abundance.Variance(),       "Abundance");
  df.Write(-1,                             "(No Data)");
  df.Write(-1,                             "(No Data)");
  df.Write(sum_genotype_depth.Variance(),  "Genotype Depth");
  df.Write(sum_generation.Variance(),      "Generation");
  df.Write(sum_neutral_metric.Variance(),  "Neutral Metric");
  df.Write(sum_lineage_label.Variance(),   "Lineage Label");
  df.Write(rave_true_replication_rate.Variance(), "True Replication Rate (based on births/update, time-averaged)");
  df.Endl();
}


void cStats::PrintDominantData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Dominant Data");
  df.WriteTimeStamp();

  df.Write(m_update,     "Update");
  df.Write(dom_merit,       "Average Merit of the Dominant Genotype");
  df.Write(dom_gestation,   "Average Gestation Time of the Dominant Genotype");
  df.Write(dom_fitness,     "Average Fitness of the Dominant Genotype");
  df.Write(dom_repro_rate,  "Repro Rate?");
  df.Write(dom_size,        "Size of Dominant Genotype");
  df.Write(dom_copied_size, "Copied Size of Dominant Genotype");
  df.Write(dom_exe_size,    "Executed Size of Dominant Genotype");
  df.Write(dom_abundance,   "Abundance of Dominant Genotype");
  df.Write(dom_births,      "Number of Births");
  df.Write(dom_breed_true,  "Number of Dominant Breed True?");
  df.Write(dom_gene_depth,  "Dominant Gene Depth");
  df.Write(dom_breed_in,    "Dominant Breed In");
  df.Write(max_fitness,     "Max Fitness?");
  df.Write(dom_genotype_id, "Genotype ID of Dominant Genotype");
  df.Write(dom_name,        "Name of the Dominant Genotype");
  df.Endl();
}

void cStats::PrintParasiteData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida Dominant Parasite Data");
  df.WriteTimeStamp();
  df.Write(m_update, "Update");
  df.Write(num_parasites, "Number of Extant Parasites");
  df.Endl();
}

void cStats::PrintPreyAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Prey Average Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_prey_fitness.Average(),        "Fitness");
  df.Write(sum_prey_gestation.Average(),      "Gestation Time");
  df.Write(sum_prey_merit.Average(),          "Merit");
  df.Write(sum_prey_creature_age.Average(),   "Creature Age");
  df.Write(sum_prey_generation.Average(),     "Generation");
  df.Write(sum_prey_size.Average(),           "Genome Length");
  df.Write(prey_entropy,                      "Total Prey Genotypic Entropy");
  
  df.Endl();
}

void cStats::PrintPredatorAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Predator Average Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_pred_fitness.Average(),        "Fitness");
  df.Write(sum_pred_gestation.Average(),      "Gestation Time");
  df.Write(sum_pred_merit.Average(),          "Merit");
  df.Write(sum_pred_creature_age.Average(),   "Creature Age");
  df.Write(sum_pred_generation.Average(),     "Generation");
  df.Write(sum_pred_size.Average(),           "Genome Length");
  df.Write(pred_entropy,                      "Total Predator Genotypic Entropy");
  
  df.Endl();
}

void cStats::PrintPreyErrorData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Prey Standard Error Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                            "Update");
  df.Write(sum_prey_fitness.StdError(),         "Fitness");
  df.Write(sum_prey_gestation.StdError(),       "Gestation Time");
  df.Write(sum_prey_merit.StdError(),           "Merit");
  df.Write(sum_prey_creature_age.StdError(),    "Creature Age");
  df.Write(sum_prey_generation.StdError(),      "Generation");
  df.Write(sum_prey_size.StdError(),            "Genome Length");
  
  df.Endl();
}

void cStats::PrintPredatorErrorData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Predator Standard Error Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                            "Update");
  df.Write(sum_pred_fitness.StdError(),         "Fitness");
  df.Write(sum_pred_gestation.StdError(),       "Gestation Time");
  df.Write(sum_pred_merit.StdError(),           "Merit");
  df.Write(sum_pred_creature_age.StdError(),    "Creature Age");
  df.Write(sum_pred_generation.StdError(),      "Generation");
  df.Write(sum_pred_size.StdError(),            "Genome Length");
  
  df.Endl();
}

void cStats::PrintPreyVarianceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Prey Variance Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                            "Update");
  df.Write(sum_prey_fitness.Variance(),         "Fitness");
  df.Write(sum_prey_gestation.Variance(),       "Gestation Time");
  df.Write(sum_prey_merit.Variance(),           "Merit");
  df.Write(sum_prey_creature_age.Variance(),    "Creature Age");
  df.Write(sum_prey_generation.Variance(),      "Generation");
  df.Write(sum_prey_size.Variance(),            "Genome Length");
  
  df.Endl();
}

void cStats::PrintPredatorVarianceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Predator Variance Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                            "Update");
  df.Write(sum_pred_fitness.Variance(),         "Fitness");
  df.Write(sum_pred_gestation.Variance(),       "Gestation Time");
  df.Write(sum_pred_merit.Variance(),           "Merit");
  df.Write(sum_pred_creature_age.Variance(),    "Creature Age");
  df.Write(sum_pred_generation.Variance(),      "Generation");
  df.Write(sum_pred_size.Variance(),            "Genome Length");
  
  df.Endl();
}

void cStats::PrintMinPreyFailedAttacks(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  if (!df.HeaderDone()) {
    df.WriteComment("Updates of individual attack that failed due to MIN_PREY config setting");
    df.WriteTimeStamp();
    df.Endl();
  }
  
  tArray<int> failure_events = m_world->GetPopulation().GetMinPreyFailedAttacks();
  if (failure_events.GetSize() > 0) {
    for (int i = 0; i < failure_events.GetSize(); i++) {
      df.WriteAnonymous(failure_events[i]);
      df.Endl();
    }
    m_world->GetPopulation().ClearMinPreyFailedAttacks();
  }
}

void cStats::PrintPreyInstructionData(const cString& filename, const cString& inst_set)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Prey org instruction execution data");
  df.WriteTimeStamp();
  
  df.Write(m_update, "Update");
  
  for (int i = 0; i < m_is_prey_exe_inst_map[inst_set].GetSize(); i++) {
    df.Write(m_is_prey_exe_inst_map[inst_set][i].Sum(), m_is_inst_names_map[inst_set][i]);
  }
  df.Endl();
}

void cStats::PrintPredatorInstructionData(const cString& filename, const cString& inst_set)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Predator org instruction execution data");
  df.WriteTimeStamp();
  
  df.Write(m_update, "Update");
  
  for (int i = 0; i < m_is_pred_exe_inst_map[inst_set].GetSize(); i++) {
    df.Write(m_is_pred_exe_inst_map[inst_set][i].Sum(), m_is_inst_names_map[inst_set][i]);
  }
  df.Endl();
}

void cStats::PrintStatsData(const cString& filename)
{
  const int genotype_change = num_genotypes - num_genotypes_last;
  const double log_ave_fid = (ave_fidelity > 0.0 && ave_fidelity != 1.0) ? -Log(ave_fidelity) : 0.0;
  const double log_dom_fid = (dom_fidelity > 0.0 && ave_fidelity != 1.0) ? -Log(dom_fidelity) : 0.0;

  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Generic Statistics Data");
  df.WriteTimeStamp();

  df.Write(m_update,          "update");
  df.Write(energy,               "average inferiority (energy)");
  df.Write(1.0 - ave_fidelity,   "ave probability of any mutations in genome");
  df.Write(1.0 - dom_fidelity,   "probability of any mutations in dom genome");
  df.Write(log_ave_fid,          "log(average fidelity)");
  df.Write(log_dom_fid,          "log(dominant fidelity)");
  df.Write(genotype_change,      "change in number of genotypes");
  df.Write(entropy,              "genotypic entropy");
  df.Write(species_entropy,      "species entropy");
  df.Write(coal_depth,           "depth of most reacent coalescence");
  df.Write(num_resamplings,      "Total number of resamplings this generation");
  df.Write(num_failedResamplings, "Total number of organisms that failed to resample this generation");

  df.Endl();
}


void cStats::PrintCountData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida count data");
  df.WriteTimeStamp();

  df.Write(m_update,         "update");
  df.Write(num_executed,           "number of insts executed this update");
  df.Write(num_creatures,          "number of organisms");
  df.Write(num_genotypes,          "number of different genotypes");
  df.Write(num_threshold,          "number of different threshold genotypes");
  df.Write(0,                      "number of different species");
  df.Write(0,                      "number of different threshold species");
  df.Write(num_lineages,           "number of different lineages");
  df.Write(num_births,             "number of births in this update");
  df.Write(num_deaths,             "number of deaths in this update");
  df.Write(num_breed_true,         "number of breed true");
  df.Write(num_breed_true_creatures, "number of breed true organisms?");
  df.Write(num_no_birth_creatures,   "number of no-birth organisms");
  df.Write(num_single_thread_creatures, "number of single-threaded organisms");
  df.Write(num_multi_thread_creatures, "number of multi-threaded organisms");
  df.Write(num_modified, "number of modified organisms");
  df.Endl();
}

void cStats::PrintMessageData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Number of organism to organism messages\n" );

  df.Write( GetUpdate(), "update" );

  cPopulation& pop = m_world->GetPopulation();
  int numDemes = pop.GetNumDemes();

	unsigned int totalMessagesSuccessfullySent(0);
	unsigned int totalMessagesDropped(0);
	unsigned int totalMessagesFailed(0);

	for( int i=0; i < numDemes; i++ ){
		totalMessagesSuccessfullySent += pop.GetDeme(i).GetMessageSuccessfullySent();
		totalMessagesDropped += pop.GetDeme(i).GetMessageDropped();
		totalMessagesFailed  += pop.GetDeme(i).GetMessageSendFailed();
	}

	df.Write(totalMessagesSuccessfullySent, "Sent successfully");
	df.Write(totalMessagesDropped, "Dropped");
	df.Write(totalMessagesFailed, "Failed");

  df.Endl();
}

void cStats::PrintInterruptData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Total number of organisms interrupted\n" );

  df.Write( GetUpdate(), "update" );

  cPopulation& pop = m_world->GetPopulation();
  int numDemes = pop.GetNumDemes();

	unsigned int totalOrgsInterrupted(0);
  unsigned int totalThreads(0);
	const int NUM_INTERRUPT_MSG_TYPES = 10;
  int interruptTypeCounts[NUM_INTERRUPT_MSG_TYPES] = {0};

	for( int i = 0; i < numDemes; ++i ){
    const cDeme & cur_deme = m_world->GetPopulation().GetDeme(i);
    for (int j = 0; j < cur_deme.GetSize(); ++j) {
      cPopulationCell& cur_cell = cur_deme.GetCell(j);
      cOrganism* org = cur_cell.GetOrganism();
      if (cur_cell.IsOccupied() == false) {
        continue;
      } else if (org->IsInterrupted()) {
        ++totalOrgsInterrupted;
        int numThreadsInOrg = org->GetHardware().GetNumThreads();
        totalThreads += numThreadsInOrg;
        for(int k = 0; k< numThreadsInOrg; ++k) {
          ++interruptTypeCounts[org->GetHardware().GetThreadMessageTriggerType(k)];
        }
      }
    }
  }

	df.Write(totalOrgsInterrupted, "Total organisms interrupted");
	df.Write(totalThreads, "Total threads");
  for (int i = 0; i < NUM_INTERRUPT_MSG_TYPES; ++i) {
    df.Write(interruptTypeCounts[i], "Interrupt Counts");
  }
  df.Endl();
}

void cStats::PrintTotalsData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.Write(m_update, "Update");
  df.Write((tot_executed+num_executed), "Total Instructions Executed");
  df.Write(num_executed, "Instructions Executed This Update");
  df.Write(tot_organisms, "Total Organisms");
  df.Write(tot_genotypes, "Total Genotypes");
  df.Write(tot_threshold, "Total Threshold");
  df.Write(0, "Total Species");
  df.Write(tot_lineages, "Total Lineages");
  df.Endl();
}

void cStats::PrintThreadsData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.Write(m_update, "Update");
  df.Write(tot_organisms, "Total Organisms");
  df.Write(m_world->GetPopulation().GetLiveOrgList().GetSize(), "Total Living Organisms");
  df.Write(m_num_threads, "Total Living Org Threads");
  df.Endl();
}



void cStats::PrintTasksData(const cString& filename)
{
	cString file = filename;

	// flag to print both tasks.dat and taskquality.dat
	if (filename == "tasksq.dat")
	{
		file = "tasks.dat";
		PrintTasksQualData("taskquality.dat");
	}

	// print tasks.dat
	cDataFile& df = m_world->GetDataFile(file);
	df.WriteComment("Avida tasks data");
	df.WriteTimeStamp();
	df.WriteComment("First column gives the current update, next columns give the number");
	df.WriteComment("of organisms that have the particular task as a component of their merit");

	df.Write(m_update,   "Update");
	for(int i = 0; i < task_last_count.GetSize(); i++) {
		df.Write(task_last_count[i], task_names[i] );
	}
	df.Endl();
}

void cStats::PrintHostTasksData(const cString& filename)
{
	cString file = filename;

	// flag to print both tasks.dat and taskquality.dat
	if (filename == "tasksq.dat")
	{
		file = "host_tasks.dat";
	}

	// print tasks.dat
	cDataFile& df = m_world->GetDataFile(file);
	df.WriteComment("Avida Host Tasks data");
	df.WriteTimeStamp();
	df.WriteComment("First column gives the current update, next columns give the number");
	df.WriteComment("of Hosts that have the particular task");

	df.Write(m_update,   "Update");
	for(int i = 0; i < tasks_host_last.GetSize(); i++) {
		df.Write(tasks_host_last[i], task_names[i] );
	}
	df.Endl();
}

void cStats::PrintParasiteTasksData(const cString& filename)
{
	cString file = filename;

	// flag to print both tasks.dat and taskquality.dat
	if (filename == "tasksq.dat")
	{
		file = "parasite_tasks.dat";
	}

	// print tasks.dat
	cDataFile& df = m_world->GetDataFile(file);
	df.WriteComment("Avida tasks data");
	df.WriteTimeStamp();
	df.WriteComment("First column gives the current update, next columns give the number");
	df.WriteComment("of Parasites that have the particular task");

	df.Write(m_update,   "Update");
	for(int i = 0; i < tasks_parasite_last.GetSize(); i++) {
		df.Write(tasks_parasite_last[i], task_names[i] );
	}
	df.Endl();
}


void cStats::PrintTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida tasks execution data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of times the particular task has been executed this update.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < task_exe_count.GetSize(); i++) {
    df.Write(task_exe_count[i], task_names[i] );
  }
  df.Endl();
}

void cStats::PrintTasksQualData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida tasks quality data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, rest give average and max task quality");
  df.Write(m_update, "Update");
  for(int i = 0; i < task_last_count.GetSize(); i++) {
    double qual = 0.0;
    if (task_last_count[i] > 0)
      qual = task_last_quality[i] / static_cast<double>(task_last_count[i]);
    df.Write(qual, task_names[i] + " Average");
    df.Write(task_last_max_quality[i], task_names[i] + " Max");
  }
  df.Endl();
}

void cStats::PrintNewTasksData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida new tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of times the particular task has newly evolved since the last time printed.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < new_task_count.GetSize(); i++) {
    df.Write(new_task_count[i], task_names[i]);
  }
  df.Endl();
  new_task_count.SetAll(0);
}

void cStats::PrintNewTasksDataPlus(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida new tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns are in sets of 3, giving the number");
  df.WriteComment("of times the particular task has newly evolved since the last time printed, then the average");
  df.WriteComment("number of tasks the parent of the organism evolving the new task performed, then the average");
  df.WriteComment("number of tasks the organism evolving the new task performed.  One set of 3 for each task");

  df.Write(m_update,   "Update");
  for (int i = 0; i < new_task_count.GetSize(); i++) {
    df.Write(new_task_count[i], task_names[i] + " - num times newly evolved");
	double prev_ave = -1;
	double cur_ave = -1;
	if (new_task_count[i]>0) {
		prev_ave = prev_task_count[i]/double(new_task_count[i]);
		cur_ave = cur_task_count[i]/double(new_task_count[i]);
	}
	df.Write(prev_ave, "ave num tasks parent performed");
	df.Write(cur_ave, "ave num tasks cur org performed");

  }
  df.Endl();
  new_task_count.SetAll(0);
  prev_task_count.SetAll(0);
  cur_task_count.SetAll(0);
}

void cStats::PrintNewReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida new reactions data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of times the particular reaction has newly evolved since the last time printed.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < new_reaction_count.GetSize(); i++) {
    df.Write(new_reaction_count[i], reaction_names[i]);
  }
  df.Endl();
  new_reaction_count.SetAll(0);
}

void cStats::PrintDynamicMaxMinData(const cString& filename)
{
	cDataFile& df = m_world->GetDataFile(filename);

	df.WriteComment("Avida dynamic max min data");
	df.WriteTimeStamp();
	df.WriteComment("First column gives the current update, 2nd and 3rd give max and min Fx");
	df.Write(m_update, "Update");
	for(int i = 0; i < task_last_count.GetSize(); i++) {
		double max = m_world->GetEnvironment().GetTask(i).GetArguments().GetDouble(1);
		double min = m_world->GetEnvironment().GetTask(i).GetArguments().GetDouble(2);
		df.Write(max, task_names[i] + " Max");
		df.Write(min, task_names[i] + " Min");
	}
	df.Endl();
}

void cStats::PrintReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida reaction data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of currently living organisms each reaction has affected.");

	df.Write(m_update,   "Update");
	for(int i = 0; i < m_reaction_last_count.GetSize(); i++) {
		df.Write(m_reaction_last_count[i], reaction_names[i]);
	}
	df.Endl();
}

void cStats::PrintCurrentReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida reaction data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of currently living organisms each reaction has affected.");

	df.Write(m_update,   "Update");
	for(int i = 0; i < m_reaction_cur_count.GetSize(); i++) {
		df.Write(m_reaction_cur_count[i], reaction_names[i]);
	}
	df.Endl();
}


void cStats::PrintReactionRewardData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida reaction data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the add bonus reward");
  df.WriteComment("currently living organisms have garnered from each reaction.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < m_reaction_last_add_reward.GetSize(); i++) {
    df.Write(m_reaction_last_add_reward[i], reaction_names[i]);
  }
  df.Endl();
}


void cStats::PrintCurrentReactionRewardData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida reaction data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the add bonus reward");
  df.WriteComment("currently living organisms have garnered from each reaction.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < m_reaction_cur_add_reward.GetSize(); i++) {
    df.Write(m_reaction_cur_add_reward[i], reaction_names[i]);
  }
  df.Endl();
}


void cStats::PrintReactionExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida reaction execution data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of times the particular reaction has been triggered this update.");

  df.Write(m_update,   "Update");
  for (int i = 0; i < m_reaction_exe_count.GetSize(); i++) {
    df.Write(m_reaction_exe_count[i], reaction_names[i]);
  }
  df.Endl();
}


void cStats::PrintResourceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Avida resource data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the quantity");
  df.WriteComment("of the particular resource at that update.");

  df.Write(m_update,   "Update");

  // Check for spatial resources if they exist total up the resource in each
  // cell and print that total.  Also call the routine to print the individual
  // maps for each spatial resource

  for (int i = 0; i < resource_count.GetSize(); i++) {
   if (resource_geometry[i] != nGeometry::GLOBAL && resource_geometry[i] != nGeometry::PARTIAL) {
         double sum_spa_resource = 0;
      for (int j = 0; j < spatial_res_count[i].GetSize(); j++) {
         sum_spa_resource += spatial_res_count[i][j];
      }
      df.Write(sum_spa_resource, resource_names[i] );
      PrintSpatialResData(filename, i);
    } else {
      df.Write(resource_count[i], resource_names[i] );
    }
  }
  df.Endl();
}

void cStats::PrintResourceLocData(const cString& filename, cAvidaContext& ctx)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Avida resource location data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the cell id");
  df.WriteComment("for center of gradient resources at that update.");
  
  df.Write(m_update,   "Update");

  const cResourceLib& resLib = m_world->GetEnvironment().GetResourceLib();
  for (int i = 0; i < resLib.GetSize(); i++) {
    if (resLib.GetResource(i)->GetGradient()) {
      df.Write(m_world->GetPopulation().GetCurrPeakX(ctx, i) + (m_world->GetPopulation().GetCurrPeakY(ctx, i) * m_world->GetConfig().WORLD_X.Get()), "CellID");
    }
  }
  df.Endl();
}

void cStats::PrintSpatialResData(const cString& filename, int i)
{

  // Write spatial resource data to a file that can easily be read into Matlab

  cString tmpfilename = "resource_";
  tmpfilename +=  resource_names[i] + ".m";
  cDataFile& df = m_world->GetDataFile(tmpfilename);
  cString UpdateStr = resource_names[i] +
                      cStringUtil::Stringf( "%07i", GetUpdate() ) + " = [ ...";

  df.WriteRaw(UpdateStr);

  int gridsize = spatial_res_count[i].GetSize();
  int xsize = m_world->GetConfig().WORLD_X.Get();

  // write grid to file

  for (int j = 0; j < gridsize; j++) {
    df.WriteBlockElement(spatial_res_count[i][j], j, xsize);
  }
  df.WriteRaw("];");
  df.Flush();
}

// @WRE: Added method for printing out visit data
void cStats::PrintCellVisitsData(const cString& filename)
{
  // Write cell visits data to a file that can easily be read into Matlab

  cString tmpfilename = "visits.m";
  cDataFile& df = m_world->GetDataFile(tmpfilename);
  cString UpdateStr = cStringUtil::Stringf( "visits%07i", GetUpdate() ) + " = [ ...";

  df.WriteRaw(UpdateStr);

  int xsize = m_world->GetConfig().WORLD_X.Get();

  for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    df.WriteBlockElement(m_world->GetPopulation().GetCell(i).GetVisits(), i, xsize);
  }

  df.WriteRaw("];");
  df.Flush();
}


void cStats::PrintTimeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida time data");
  df.WriteTimeStamp();

  df.Write(m_update,              "update");
  df.Write(avida_time,               "avida time");
  df.Write(sum_generation.Average(), "average generation");
  df.Write(num_executed,             "num_executed?");
  df.Endl();
}


//@MRR Add additional time information
void cStats::PrintExtendedTimeData(const cString& filename)
{
	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteTimeStamp();
	df.Write(m_update, "update");
	df.Write(avida_time, "avida time");
	df.Write(num_executed, "num_executed");
	df.Write(tot_organisms, "num_organisms");
	df.Endl();
}

void cStats::PrintMutationRateData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida copy mutation rate data");
  df.WriteTimeStamp();

  df.Write(m_update, "Update");
  df.Write(sum_copy_mut_rate.Mean(), "Average copy mutation rate");
  df.Write(sum_copy_mut_rate.Variance(), "Variance in copy mutation rate");
  df.Write(sum_copy_mut_rate.StdDeviation(), "Standard Deviation in copy mutation rate");
  df.Write(sum_copy_mut_rate.Skewness(), "Skew in copy mutation rate");
  df.Write(sum_copy_mut_rate.Kurtosis(), "Kurtosis in copy mutation rate");

  df.Write(sum_log_copy_mut_rate.Mean(), "Average log(copy mutation rate)");
  df.Write(sum_log_copy_mut_rate.Variance(), "Variance in log(copy mutation rate)");
  df.Write(sum_log_copy_mut_rate.StdDeviation(), "Standard Deviation in log(copy mutation rate)");
  df.Write(sum_log_copy_mut_rate.Skewness(), "Skew in log(copy mutation rate)");
  df.Write(sum_log_copy_mut_rate.Kurtosis(), "Kurtosis in log(copy mutation rate)");
  df.Endl();

}


void cStats::PrintDivideMutData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida divide mutation rate data");
  df.WriteTimeStamp();

  df.Write(m_update, "Update");
  df.Write(sum_div_mut_rate.Mean(), "Average divide mutation rate");
  df.Write(sum_div_mut_rate.Variance(), "Variance in divide mutation rate");
  df.Write(sum_div_mut_rate.StdDeviation(), "Standard Deviation in divide mutation rate");
  df.Write(sum_div_mut_rate.Skewness(), "Skew in divide mutation rate");
  df.Write(sum_div_mut_rate.Kurtosis(), "Kurtosis in divide mutation rate");

  df.Write(sum_log_div_mut_rate.Mean(), "Average log(divide mutation rate)");
  df.Write(sum_log_div_mut_rate.Variance(), "Variance in log(divide mutation rate)");
  df.Write(sum_log_div_mut_rate.StdDeviation(), "Standard Deviation in log(divide mutation rate)");
  df.Write(sum_log_div_mut_rate.Skewness(), "Skew in log(divide mutation rate)");
  df.Write(sum_log_div_mut_rate.Kurtosis(), "Kurtosis in log(divide mutation rate)");
  df.Endl();
}

void cStats::PrintInstructionData(const cString& filename, const cString& inst_set)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida instruction execution data");
  df.WriteTimeStamp();

  df.Write(m_update, "Update");

  for (int i = 0; i < m_is_exe_inst_map[inst_set].GetSize(); i++) {
    df.Write(m_is_exe_inst_map[inst_set][i].Sum(), m_is_inst_names_map[inst_set][i]);
  }

  df.Endl();
}

void cStats::PrintMarketData(const cString& filename)
{
	cDataFile& df = m_world->GetDataFile(filename);

	df.WriteComment( "Avida market data\n" );
	df.WriteComment("cumulative totals since the last update data was printed" );
	df.WriteTimeStamp();
	df.Write( GetUpdate(), "update" );
	df.Write( num_bought, "num bought" );
	df.Write( num_sold, "num sold" );
	df.Write(num_used, "num used" );
	df.Write(num_own_used, "num own used" );
	num_bought = num_sold = num_used = num_own_used = 0;
  df.Endl();
}

void cStats::PrintSenseData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Avida sense instruction usage\n" );
  df.WriteComment("total number of organisms whose parents executed sense instructions with given labels" );

  df.Write( GetUpdate(), "update" );

  for( int i=0; i < sense_last_count.GetSize(); i++ ){
    df.Write(sense_last_count[i], sense_names[i]);
  }
  df.Endl();
}

void cStats::PrintSenseExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Avida sense instruction usage\n" );
  df.WriteComment("total number of sense instructions executed by the parents of current organisms with given labels" );

  df.Write( GetUpdate(), "update" );

  for( int i=0; i < sense_last_exe_count.GetSize(); i++ ){
    df.Write(sense_last_exe_count[i], sense_names[i]);
  }
  df.Endl();
}

void cStats::PrintInternalTasksData(const cString& filename)
{
	cString file = filename;

	// flag to print both in_tasks.dat and in_taskquality.dat
	if (filename == "in_tasksq.dat")
	{
		file = "in_tasks.dat";
		PrintInternalTasksQualData("in_taskquality.dat");
	}

	// print in_tasks.dat
	cDataFile& df = m_world->GetDataFile(file);
	df.WriteComment("Avida tasks data: tasks performed with internal resources");
	df.WriteTimeStamp();
	df.WriteComment("First column gives the current update, next columns give the number");
	df.WriteComment("of organisms that have the particular task, performed with internal resources, ");
	df.WriteComment("as a component of their merit");

	df.Write(m_update,   "Update");
	for(int i = 0; i < task_internal_last_count.GetSize(); i++) {
		df.Write(task_internal_last_count[i], task_names[i] );
	}
	df.Endl();
}

void cStats::PrintInternalTasksQualData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida tasks quality data: tasks performed using internal resources");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, rest give average and max task quality ");
  df.WriteComment("for those tasks performed using internal resources");
  df.Write(m_update, "Update");
  for(int i = 0; i < task_internal_last_count.GetSize(); i++) {
    double qual = 0.0;
    if (task_internal_last_count[i] > 0)
      qual = task_internal_last_quality[i] / static_cast<double>(task_internal_last_count[i]);
    df.Write(qual, task_names[i] + " Average");
    df.Write(task_internal_last_max_quality[i], task_names[i] + " Max");
  }
  df.Endl();
}

void cStats::PrintSleepData(const cString& filename){
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Number of organisms sleeping\n" );
  df.WriteComment("total number of organisms sleeping" );

  df.Write( GetUpdate(), "update" );

  cPopulation& pop = m_world->GetPopulation();
  int numDemes = pop.GetNumDemes();

  for( int i=0; i < numDemes; i++ ){
    df.Write(pop.GetDeme(i).GetSleepingCount(), cStringUtil::Stringf("DemeID %d", i));
  }
  df.Endl();
}

void cStats::PrintCompetitionData(const cString& filename){
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment( "Competition results\n" );
  df.WriteComment( "results of the current competitions" );

  df.Write( GetUpdate(), "update" );
  df.Write( avg_competition_fitness, "average competition fitness" );
  df.Write( min_competition_fitness, "min competition fitness" );
  df.Write( max_competition_fitness, "max competition fitness" );
  df.Write( avg_competition_copied_fitness, "average copied fitness" );
  df.Write( min_competition_copied_fitness, "min copied fitness" );
  df.Write( max_competition_copied_fitness, "max copied fitness" );
  df.Write( num_orgs_replicated, "number of organisms copied" );

  // Only print trial info if there were multiple trials.
  if (avg_trial_fitnesses.GetSize() > 1)
  {
    for( int i=0; i < avg_trial_fitnesses.GetSize(); i++ ){
      df.Write(avg_trial_fitnesses[i], cStringUtil::Stringf("trial.%d fitness", i));
    }
  }
  df.Endl();
}


/*! This method is called whenever an organism successfully sends a message.  Success,
in this case, means that the message has been delivered to the receive buffer of
the organism that this message was sent to. */
void cStats::SentMessage(const cOrgMessage& msg)
{
  // Check to see if this message matches any of our predicates.
  for(message_pred_ptr_list::iterator i=m_message_predicates.begin(); i!=m_message_predicates.end(); ++i) {
    (**i)(msg); // Predicate is responsible for tracking info about messages.
  }
}


/*! This method adds a message predicate to the list of all predicates.  Each predicate
in the list is evaluated for every sent message.

NOTE: cStats does NOT own the predicate pointer!  (It DOES NOT delete them!)
*/
void cStats::AddMessagePredicate(cOrgMessagePredicate* predicate)
{
  m_message_predicates.push_back(predicate);
}

void cStats::RemoveMessagePredicate(cOrgMessagePredicate* predicate)
{
  for(message_pred_ptr_list::iterator iter = m_message_predicates.begin(); iter != m_message_predicates.end(); iter++) {
    if((*iter) == predicate) {
      m_message_predicates.erase(iter);
      return;
    }
  }
}


/*! This method adds a movement predicate to the list of all movement predicates.  Each predicate
 * in the list is evaluated for every organism movement.
 *
 * NOTE: cStats does NOT own the predicate pointer!  (It DOES NOT delete them!)
 * */
void cStats::AddMovementPredicate(cOrgMovementPredicate* predicate)
{
  m_movement_predicates.push_back(predicate);
}

/*! This method is called whenever an organism moves.*/
void cStats::Move(cOrganism& org) {
  // Check to see if this message matches any of our predicates.
  for(movement_pred_ptr_list::iterator i=m_movement_predicates.begin();
      i!=m_movement_predicates.end(); ++i) {
    (**i)(org); // Predicate is responsible for tracking info about movement.
  }
}

// deme predicate stats
void cStats::IncEventCount(int x, int y) {
  relative_pos_event_count.ElementAt(x,y)++;
}

void cStats::IncPredSat(int cell_id) {
  cPopulation& pop = m_world->GetPopulation();
  int deme_id = pop.GetCell(cell_id).GetDemeID();
  std::pair<int, int> pos = pop.GetDeme(deme_id).GetCellPosition(cell_id);
  relative_pos_pred_sat.ElementAt(pos.first, pos.second)++;
}

void cStats::AddDemeResourceThresholdPredicate(cString& name) {
	demeResourceThresholdPredicateMap[name] = 0;
}

void cStats::IncDemeResourceThresholdPredicate(cString& name) {
	++demeResourceThresholdPredicateMap[name];
}

void cStats::PrintDemeResourceThresholdPredicate(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme resource threshold predicate data");
	df.WriteComment("Number of deme reproduced by a specific threshold since last update that data was printed");
  df.WriteTimeStamp();

	if(demeResourceThresholdPredicateMap.size() > 0) {
		df.Write(GetUpdate(), "Update [update]");
		for(map<cString, int>::iterator iter = demeResourceThresholdPredicateMap.begin(); iter != demeResourceThresholdPredicateMap.end(); ++iter) {
			df.Write(iter->second, iter->first);
			iter->second = 0;
			assert(iter->second == 0 && demeResourceThresholdPredicateMap[iter->first] == 0);
		}
		df.Endl();
	}
}

/*! This method prints information contained within all active message predicates.

 Each row of the data file has the following format:
   update predicate_name predicate_data...
 */
void cStats::PrintPredicatedMessages(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteColumnDesc("update [update]");
  df.WriteColumnDesc("predicate name: [pname]");
  df.WriteColumnDesc("predicate data: [pdata]");
  df.FlushComments();

  std::ofstream& out = df.GetOFStream();
  for(message_pred_ptr_list::iterator i=m_message_predicates.begin();
    i!=m_message_predicates.end(); ++i) {
      (*i)->Print(GetUpdate(), out);
      (*i)->Reset();
  }
//  df.Endl();
}

void cStats::PrintPredSatFracDump(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment( "Displays the fraction of events detected in cell since last print.\n" );
  df.FlushComments();
  cString UpdateStr = cStringUtil::Stringf( "%07i", GetUpdate() ) + " = [ ...";
  df.WriteRaw(UpdateStr);

  int rows = relative_pos_pred_sat.GetNumRows();
  int cols = relative_pos_pred_sat.GetNumCols();
  for (int x = 0; x < rows; x++) {
    for (int y = 0; y < cols; y++) {
      double data;
      if(relative_pos_event_count.ElementAt(x,y) == 0) {
        data = 0.0;
      } else {
        data = (double) relative_pos_pred_sat.ElementAt(x,y) / (double) relative_pos_event_count.ElementAt(x,y);
      }
      df.WriteBlockElement(data, x*cols+y, cols);
    }
  }
  df.WriteRaw("];");
  df.Endl();

  relative_pos_pred_sat.SetAll(0);
  relative_pos_event_count.SetAll(0);
}

void cStats::DemePreReplication(cDeme& source_deme, cDeme& target_deme)
{
  ++m_deme_num_repls;
  ++m_total_deme_num_repls;
  m_deme_gestation_time.Add(source_deme.GetAge());
  m_deme_births.Add(source_deme.GetBirthCount());
  m_deme_merit.Add(source_deme.GetHeritableDemeMerit().GetDouble());
  m_deme_generation.Add(source_deme.GetGeneration());
  m_deme_density.Add(source_deme.GetDensity());

  if(source_deme.isTreatable()) {
    ++m_deme_num_repls_treatable;
    m_deme_gestation_time_treatable.Add(source_deme.GetAge());
    m_deme_births_treatable.Add(source_deme.GetBirthCount());
    m_deme_merit_treatable.Add(source_deme.GetHeritableDemeMerit().GetDouble());
    m_deme_generation_treatable.Add(source_deme.GetGeneration());
    m_deme_density_treatable.Add(source_deme.GetDensity());
  } else {
    ++m_deme_num_repls_untreatable;
    m_deme_gestation_time_untreatable.Add(source_deme.GetAge());
    m_deme_births_untreatable.Add(source_deme.GetBirthCount());
    m_deme_merit_untreatable.Add(source_deme.GetHeritableDemeMerit().GetDouble());
    m_deme_generation_untreatable.Add(source_deme.GetGeneration());
    m_deme_density_untreatable.Add(source_deme.GetDensity());
  }
  
  /* Track the number of mutations that have occured to the germline as the result of damage resulting from performing metabolic work. Only add to stats if there is a germline... */
  double n_mut = source_deme.GetAveGermMut();
  if (n_mut >= 0) {
    m_ave_germ_mut.push_back(n_mut);
    m_ave_non_germ_mut.push_back(source_deme.GetAveNonGermMut());
    m_ave_germ_size.push_back(source_deme.GetGermlinePercent());
  }
}


/*! This method is a generic hook for post-deme-replication stat tracking.  We
currently only track the genotype ids of all the founders of each deme in the population.
Note that we capture genotype ids at the time of deme replication, so we unfortunately
lose the ancestral deme founders.
*/
void cStats::DemePostReplication(cDeme& source_deme, cDeme& target_deme)
{
  m_deme_founders[target_deme.GetID()] = target_deme.GetGenotypeIDs();
}


/*! Called immediately prior to germline replacement.
*/
void cStats::GermlineReplication(cGermline& source_germline, cGermline& target_germline)
{
  m_germline_generation.Add(source_germline.Size());
}






/*! Print statistics related to deme replication.  Currently only prints the
number of deme replications since the last time PrintDemeReplicationData was
invoked.
*/
void cStats::PrintDemeReplicationData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme replication data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write(m_deme_num_repls, "Number of deme replications [numrepl]");
  df.Write(m_deme_gestation_time.Average(), "Mean deme gestation time [gesttime]");
  df.Write(m_deme_births.Average(), "Mean number of births within replicated demes [numbirths]");
  df.Write(m_deme_merit.Average(), "Mean heritable merit of replicated demes [merit]");
  df.Write(m_deme_generation.Average(), "Mean generation of replicated demes [generation]");
  df.Write(m_deme_density.Average(), "Mean density of replicated demes [density]");
  df.Endl();

  m_deme_num_repls = 0;
  m_deme_gestation_time.Clear();
  m_deme_births.Clear();
  m_deme_merit.Clear();
  m_deme_generation.Clear();
	m_deme_density.Clear();
  
}

/*! Print statistics related to whether or not the demes are sequestering the germline...   Currently prints information from the last 100 deme replications events.
 */
void cStats::PrintDemeGermlineSequestration(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Avida deme germline sequestration data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  
  while(m_ave_germ_mut.size()>100) {
		m_ave_germ_mut.pop_front();
	}
  while(m_ave_non_germ_mut.size()>100) {
		m_ave_non_germ_mut.pop_front();
	}
  while(m_ave_germ_size.size()>100) {
		m_ave_germ_size.pop_front();
	}
  
  if(m_ave_germ_mut.empty()) {
		df.Write(0.0, "Mean number of mutations to germline [meangermmut]"); 
    df.Write(0.0, "Mean number of mutations to non-germline orgs [meannongermmut]");
    df.Write(0.0, "Mean size of germ line [meangermsize]");

	} 
  else {
    df.Write(std::accumulate(m_ave_germ_mut.begin(), m_ave_germ_mut.end(), 0.0)/m_ave_germ_mut.size(), "Mean number of mutations to germline [meangermmut]");
    df.Write(std::accumulate(m_ave_non_germ_mut.begin(), m_ave_non_germ_mut.end(), 0.0)/m_ave_non_germ_mut.size(), "Mean number of mutations to non-germline orgs [meannongermmut]");	
    df.Write(std::accumulate(m_ave_germ_size.begin(), m_ave_germ_size.end(), 0.0)/m_ave_germ_size.size(), "Mean size of germ line [meangermsize]");
  }
   
  df.Endl();

}

/*! Print statistics related to whether or not the demes are sequestering the germline...   
 Currently prints information for each org in each deme.
 */
void cStats::PrintDemeOrgGermlineSequestration(const cString& filename)
{  
  
  cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("Cell data per udpate.");
	df.WriteTimeStamp();
  
  cPopulation& pop = m_world->GetPopulation();
	static const int numDemes = pop.GetNumDemes();
  
  for(int i = 0; i < numDemes; ++i) {
    cDeme& deme = pop.GetDeme(i);
    df.Write(GetUpdate(), "Update [update]");
    df.Write(deme.GetDemeID(), "Deme ID for cell [demeid]");
    df.Write(deme.GetTotalResourceAmountConsumed(), "Deme resources consumed [demeres]");
    
    tArray<int> react_count = deme.GetReactionCount(); 
    for (int k=0; k<react_count.GetSize(); ++k){
      react_count[k] = 0;
    }
    
    int numGerm = 0; 
    int numMut = 0; 
    int numPresent = 0;
    
    for (int j=0; j<deme.GetSize(); ++j) {

      cPopulationCell& cell = deme.GetCell(j);
      if (cell.IsOccupied()) {
        cOrganism* o = cell.GetOrganism();
        int isGerm = 0;
        if (o->IsGermline()) isGerm = 1;
                      
        df.Write(isGerm, "Org is germ line [isgerm]");
        df.Write(o->GetNumOfPointMutationsApplied(), "Number of point mutations [numPoint]");
        if (isGerm) numGerm++; 
        numMut += o->GetNumOfPointMutationsApplied();
        numPresent++;
        
         tArray<int> org_react_count = o->GetPhenotype().GetCumulativeReactionCount();
         for (int k=0; k<org_react_count.GetSize(); ++k){
           react_count[k] += org_react_count[k];
         }
      }
        // Cell is not occuppied.
      else {
        df.Write(2, "Org is germ line [isgerm]");
        df.Write(0, "Number of point mutations [numPoint]");
      }
    }
    for (int k=0; k<react_count.GetSize(); ++k){
      df.Write(react_count[k], "reaction");
    }
    df.Write(numGerm, "numGerm");
    df.Write(numPresent, "numPresent");
    df.Write(numMut, "numMut");

    
    
    df.Endl();
	}
}


/*! Print the genotype ID and genotypes of the founders of recently born demes that use germline method = 3, 
 where the organisms flag themselves as part of the germline.
 
 Only deme "births" (i.e., due to deme replication) are tracked; the ancestral deme founders are lost.  
 The update column is the update at which this method executes, not the time at which the given deme was born.
 */
void cStats::PrintDemeGLSFounders(const cString& filename){


    cDataFile& df = m_world->GetDataFile(filename);
    
    df.WriteComment("Avida gls deme founder data.");
    df.WriteTimeStamp();
    df.WriteColumnDesc("Update [update]");
    df.WriteColumnDesc("Soure Deme ID [sdemeid]");
    df.WriteColumnDesc("Target Deme ID [tdemeid]");
    df.WriteColumnDesc("Number of founders [size]");
    df.WriteColumnDesc("{target genotype ID, target genome... founder 0, ...}");
    df.FlushComments();
    
    std::ofstream& out = df.GetOFStream();
   
   //  typedef std::map<std::pair<int, int>, std::vector<std::pair<int, std::string> > > t_gls_founder_map;

    for(t_gls_founder_map::iterator i=m_gls_deme_founders.begin(); i!=m_gls_deme_founders.end(); ++i) {
      out << GetUpdate() << " " << i->first.first << " " << i->first.second << " " << i->second.size();
      for(std::vector<std::pair<int, std::string> >::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
        out << " " << (*j).first << " " << (*j).second; 
//        out << " " << *j;
      }
      df.Endl();
    }
    m_gls_deme_founders.clear();

}

//! Track GLS Deme Founder Data
void cStats::TrackDemeGLSReplication(int source_deme_id, int target_deme_id,   std::vector<std::pair<int, std::string> > founders){
  m_gls_deme_founders[make_pair(source_deme_id, target_deme_id)] = founders;
}





/*! Print statistics related to deme replication.  Currently only prints the
 number of deme replications since the last time PrintDemeReplicationData was
 invoked.
 */
void cStats::PrintDemeTreatableReplicationData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme replication data for treatable deme");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write(m_deme_num_repls_treatable, "Number of deme replications [numrepl]");
  df.Write(m_deme_gestation_time_treatable.Average(), "Mean deme gestation time [gesttime]");
  df.Write(m_deme_births_treatable.Average(), "Mean number of births within replicated demes [numbirths]");
  df.Write(m_deme_merit_treatable.Average(), "Mean heritable merit of replicated demes [merit]");
  df.Write(m_deme_generation_treatable.Average(), "Mean generation of replicated demes [generation]");
	df.Write(m_deme_density_treatable.Average(), "Mean density of replicated demes [density]");

  df.Endl();

  m_deme_num_repls_treatable = 0;
  m_deme_gestation_time_treatable.Clear();
  m_deme_births_treatable.Clear();
  m_deme_merit_treatable.Clear();
  m_deme_generation_treatable.Clear();
	m_deme_density_treatable.Clear();
}

/*! Print statistics related to deme replication.  Currently only prints the
 number of deme replications since the last time PrintDemeReplicationData was
 invoked.
 */
void cStats::PrintDemeUntreatableReplicationData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme replication data for untreatable deme");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write(m_deme_num_repls_untreatable, "Number of deme replications [numrepl]");
  df.Write(m_deme_gestation_time_untreatable.Average(), "Mean deme gestation time [gesttime]");
  df.Write(m_deme_births_untreatable.Average(), "Mean number of births within replicated demes [numbirths]");
  df.Write(m_deme_merit_untreatable.Average(), "Mean heritable merit of replicated demes [merit]");
  df.Write(m_deme_generation_untreatable.Average(), "Mean generation of replicated demes [generation]");
	df.Write(m_deme_density_untreatable.Average(), "Mean density of replicated demes [density]");

  df.Endl();

  m_deme_num_repls_untreatable = 0;
  m_deme_gestation_time_untreatable.Clear();
  m_deme_births_untreatable.Clear();
  m_deme_merit_untreatable.Clear();
  m_deme_generation_untreatable.Clear();
	m_deme_density_untreatable.Clear();
}


void cStats::PrintDemeTreatableCount(const cString& filename)
{
  cPopulation& pop = m_world->GetPopulation();
	static const int numDemes = pop.GetNumDemes();
	int treatable(0);
	int untreatable(0);
	for(int i = 0; i < numDemes; ++i) {
		if(pop.GetDeme(i).isTreatable())
			++treatable;
		else
			++untreatable;
	}

	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme replication data for untreatable deme");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write(treatable, "Number demes treatable");
  df.Write(untreatable, "Number demes untreatable");
  df.Write(static_cast<double>(treatable)/static_cast<double>(untreatable), "Treatable:untreatable ratio");

  df.Endl();
}

void cStats::PrintGermlineData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida germline data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write(m_germline_generation.Average(), "Mean germline generation of replicated germlines [replgen]");
  df.Endl();

  m_germline_generation.Clear();
}


/*! Print the genotype IDs of the founders of recently born demes.

Prints only the most recent set of founding genotype ids for each deme.  If a deme was replaced multiple
times since the last time this method ran, only the most recent is printed.  Only deme "births" (i.e., due
to deme replication) are tracked; the ancestral deme founders are lost.  The update column is the update
at which this method executes, not the time at which the given deme was born.
*/
void cStats::PrintDemeFoundersData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme founder data.");
  df.WriteTimeStamp();
  df.WriteColumnDesc("Update [update]");
  df.WriteColumnDesc("Deme ID [demeid]");
  df.WriteColumnDesc("Number of founders [size]");
  df.WriteColumnDesc("{Genotype ID of founder 0, ...}");
  df.FlushComments();

  std::ofstream& out = df.GetOFStream();
  for(t_founder_map::iterator i=m_deme_founders.begin(); i!=m_deme_founders.end(); ++i) {
    out << GetUpdate() << " " << i->first << " " << i->second.size();
    for(std::vector<int>::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
      out << " " << *j;
    }
    df.Endl();
  }
  m_deme_founders.clear();
}


void cStats::PrintPerDemeTasksData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of organisms that have the particular task as a component of their merit");
  df.WriteComment("in a particular deme");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  df.Write(m_update, "Update");
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for(int j = 0; j < num_tasks; j++) {
      df.Write( (deme.GetLastTaskExeCount()[j] > 0), cStringUtil::Stringf("%i.", i) + task_names[j] );
    }
  }
  df.Endl();
}


void cStats::PrintPerDemeTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme tasks exe data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of times a task has contributed to the merit of all organisms");
  df.WriteComment("in a particular deme");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  df.Write(m_update, "Update");
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for(int j = 0; j < num_tasks; j++) {
      df.Write( deme.GetLastTaskExeCount()[j], cStringUtil::Stringf("%i.", i) + task_names[j] );
    }
  }
  df.Endl();
}


void cStats::PrintAvgDemeTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  const int num_tasks = m_world->GetEnvironment().GetNumTasks();
  cIntSum tasksum;

  df.WriteComment("Avida average deme tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column is the update, remaining columns are the average number of times");
  df.WriteComment("each task has been executed by the demes");
  df.WriteComment(cStringUtil::Stringf("Data based on %i demes and %i tasks", num_demes, num_tasks));

  df.Write(m_update, "Update");

  for(int t = 0; t < num_tasks; t++) {
    tasksum.Clear();

    for(int d = 0; d < num_demes; d++) {
      cDeme& deme = m_world->GetPopulation().GetDeme(d);
      tasksum.Add(deme.GetLastTaskExeCount()[t]);
    }
    df.Write(tasksum.Average(), task_names[t]);
  }
  df.Endl();
}


void cStats::PrintAvgTreatableDemeTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  const int num_tasks = m_world->GetEnvironment().GetNumTasks();
  cIntSum tasksum;

  df.WriteComment("Avida average tasks data for treatable demes");
  df.WriteTimeStamp();
  df.WriteComment("First column is the update, remaining columns are the average number of times");
  df.WriteComment("each task has been executed by treatable demes");
  df.WriteComment(cStringUtil::Stringf("Data based on %i demes and %i tasks", num_demes, num_tasks));

  df.Write(m_update, "Update");

  for(int t = 0; t < num_tasks; t++) {
    tasksum.Clear();

    for(int d = 0; d < num_demes; d++) {
      cDeme& deme = m_world->GetPopulation().GetDeme(d);
      if(deme.isTreatable()) {
        tasksum.Add(deme.GetLastTaskExeCount()[t]);
      }
    }
    df.Write(tasksum.Average(), task_names[t]);
  }
  df.Endl();
}


void cStats::PrintAvgUntreatableDemeTasksExeData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  const int num_tasks = m_world->GetEnvironment().GetNumTasks();
  cIntSum tasksum;

  df.WriteComment("Avida average tasks data for untreatable demes");
  df.WriteTimeStamp();
  df.WriteComment("First column is the update, remaining columns are the average number of times");
  df.WriteComment("each task has been executed by untreatable demes");
  df.WriteComment(cStringUtil::Stringf("Data based on %i demes and %i tasks", num_demes, num_tasks));

  df.Write(m_update, "Update");

  for(int t = 0; t < num_tasks; t++) {
    tasksum.Clear();

    for(int d = 0; d < num_demes; d++) {
      cDeme& deme = m_world->GetPopulation().GetDeme(d);
      if(!deme.isTreatable()) {
        tasksum.Add(deme.GetLastTaskExeCount()[t]);
      }
    }
    df.Write(tasksum.Average(), task_names[t]);
  }
  df.Endl();
}


void cStats::PrintPerDemeReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme reactions data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of currently living organisms each reaction has affected.");

  const int num_reactions = m_world->GetEnvironment().GetReactionLib().GetSize();

  df.Write(m_update,   "Update");
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for(int j = 0; j < num_reactions; j++) {
      df.Write( deme.GetLastReactionCount()[j], cStringUtil::Stringf("%i.", i) + m_world->GetEnvironment().GetReactionLib().GetReaction(j)->GetName()  );
    }
  }
  df.Endl();
}

void cStats::PrintDemeTasksData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of organisms per deme that had the given task as a component of their merit");
  df.WriteComment("during the lifetime of the deme");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  tArray<int> deme_tasks;
  deme_tasks.ResizeClear(num_tasks);
  deme_tasks.SetAll(num_tasks);
  int occupied_demes = 0;
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    if (!deme.IsEmpty()) {
      occupied_demes++;
      for(int j = 0; j < num_tasks; j++) {
        deme_tasks[j] += static_cast<int>(deme.GetLastTaskExeCount()[j] > 0);
      }
    }
  }

  df.Write(m_update,   "Update");
  for(int j = 0; j < num_tasks; j++) {
    df.Write( static_cast<double>(deme_tasks[j]) / static_cast<double>(occupied_demes), task_names[j] );
  }
  df.Endl();
}

void cStats::PrintDemeTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme tasks exe data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of times per deme that a given task counted as a component of an");
  df.WriteComment("organisms's merit during the lifetime of the deme");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  tArray<int> deme_tasks;
  deme_tasks.ResizeClear(num_tasks);
  deme_tasks.SetAll(num_tasks);
  int occupied_demes = 0;
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    if (!deme.IsEmpty()) {
      occupied_demes++;
      for(int j = 0; j < num_tasks; j++) {
        deme_tasks[j] += deme.GetLastTaskExeCount()[j];
      }
    }
  }

  df.Write(m_update,   "Update");
  for(int j = 0; j < num_tasks; j++) {
    df.Write( static_cast<double>(deme_tasks[j]) / static_cast<double>(occupied_demes), task_names[j] );
	}
  df.Endl();
}

void cStats::PrintDemeReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme reactions data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of times each reaction has affected a deme.");

  const int num_reactions = m_world->GetEnvironment().GetReactionLib().GetSize();

  tArray<int> deme_reactions;
  deme_reactions.ResizeClear(num_reactions);
  deme_reactions.SetAll(0);
  int occupied_demes = 0;
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    if (!deme.IsEmpty()) {
      occupied_demes++;
      for(int j = 0; j < num_reactions; j++) {
        deme_reactions[j] += deme.GetLastReactionCount()[j];
      }
    }
  }

  df.Write(m_update,   "Update");
  for(int j = 0; j < num_reactions; j++) {
    df.Write( static_cast<double>(deme_reactions[j]) / static_cast<double>(occupied_demes), m_world->GetEnvironment().GetReactionLib().GetReaction(j)->GetName() );
  }
  df.Endl();
}

void cStats::PrintDemeOrgTasksData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme org tasks data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of organisms that have the particular task as a component of their merit");
  df.WriteComment("in a particular deme when the deme last divided.");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  df.Write(m_update,   "Update");
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for(int j = 0; j < num_tasks; j++) {
      df.Write( deme.GetLastOrgTaskCount()[j], cStringUtil::Stringf("%i.", i) + task_names[j] );
    }
  }
  df.Endl();
}

void cStats::PrintDemeOrgTasksExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme org tasks exe data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, next columns give the number");
  df.WriteComment("of times a task has contributed to the merit of all organisms");
  df.WriteComment("in a particular deme when the deme last divided.");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();

  df.Write(m_update,   "Update");
  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for(int j = 0; j < num_tasks; j++) {
      df.Write( deme.GetLastOrgTaskExeCount()[j], cStringUtil::Stringf("%i.", i) + task_names[j] );
    }
  }

  df.Endl();
}

void cStats::PrintDemeCurrentTaskExeData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme current task exe data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives update number, next columns give the number");
  df.WriteComment("of times a given task has been executed in a given deme by");
  df.WriteComment("some organism in that deme.");

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();
  df.Write(m_update, "Update");
  for (int deme_num=0; deme_num < m_world->GetPopulation().GetNumDemes(); ++deme_num) {
    cDeme& deme = m_world->GetPopulation().GetDeme(deme_num);
    for (int task_num=0; task_num < num_tasks; task_num++) {
      df.Write(	deme.GetCurTaskExeCount()[task_num],
        cStringUtil::Stringf("%i.", deme_num)+task_names[task_num]);
    }
  }

  df.Endl();
}

void cStats::PrintCurrentTaskCounts(const cString& filename)
{
  ofstream& fp = m_world->GetDataFileOFStream(filename);
  fp << "Update " << m_world->GetStats().GetUpdate() << ":" << endl;
  for (int y = 0; y < m_world->GetPopulation().GetWorldY(); y++) {
    for (int x = 0; x < m_world->GetPopulation().GetWorldX(); x++) {
      cPopulationCell& cell = m_world->GetPopulation().GetCell(y * m_world->GetPopulation().GetWorldX() + x);
      if (cell.IsOccupied()) {
        fp << cell.GetOrganism()->GetPhenotype().GetCurTaskCount()[0] << "\t";
      } else {
        fp << "---\t";
      }
    }
    fp << endl;
  }
  fp << endl;
}

void cStats::PrintDemeOrgReactionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida deme org reactions data");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("of currently living organisms each reaction has affected");
  df.WriteComment("in a particular deme when the deme last divided.");

  const int num_reactions = m_world->GetEnvironment().GetReactionLib().GetSize();

  df.Write(m_update, "Update");
  for (int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    for (int j = 0; j < num_reactions; j++) {
      df.Write(deme.GetLastOrgReactionCount()[j], cStringUtil::Stringf("%i.", i) + m_world->GetEnvironment().GetReactionLib().GetReaction(j)->GetName());
    }
  }
  df.Endl();
}

//@JJB**
void cStats::PrintDemesTasksData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida current tasks done by each deme");
  df.WriteTimeStamp();

  const int num_tasks = m_world->GetEnvironment().GetNumTasks();
  df.Write(m_update, "Update");
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDeme& deme = m_world->GetPopulation().GetDeme(deme_id);
    for (int task_id = 0; task_id < num_tasks; task_id++) {
      df.Write(deme.GetTaskCount()[task_id], cStringUtil::Stringf("%i.", deme_id) + task_names[task_id]);
    }
  }
  df.Endl();
}

//@JJB**
void cStats::PrintDemesReactionsData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida current reactions done by each deme");
  df.WriteTimeStamp();

  const int num_reactions = m_world->GetEnvironment().GetReactionLib().GetSize();
  df.Write(m_update, "Update");
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDeme& deme = m_world->GetPopulation().GetDeme(deme_id);
    for (int reaction_id = 0; reaction_id < num_reactions; reaction_id++) {
      df.Write(deme.GetReactionCount()[reaction_id], cStringUtil::Stringf("%i.", deme_id) + m_world->GetEnvironment().GetReactionLib().GetReaction(reaction_id)->GetName());
    }
  }
  df.Endl();
}

//@JJB**
void cStats::PrintDemesFitnessData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida competition fitness for each deme");
  df.WriteTimeStamp();

  df.Write(m_update, "Update");
  const int num_demes = m_world->GetPopulation().GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    //df.Write(m_deme_fitness[deme_id], cStringUtil::Stringf("%i.Fitness", deme_id));
  }
  df.Endl();
}

void cStats::PrintPerDemeGenPerFounderData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida org generations between deme founders");
  df.WriteTimeStamp();
  df.WriteComment("First column gives the current update, all further columns give the number");
  df.WriteComment("number of generations that passed between the parent and current deme's founders");

  df.Write(m_update, "Update");
  for (int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
    double val = deme.GetGenerationsPerLifetime();
    if (deme.IsEmpty()) val = -1;
    df.Write(val, cStringUtil::Stringf("deme.%i", i));
  }
  df.Endl();
}

void cStats::PrintDemeMigrationSuicidePoints(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Avida average stats");
  df.WriteTimeStamp();


  df.Write(m_update, "Update");
  double max_points = 0;
  double min_points = -1;
  double total_points = 0;
  double temp_points = 0;
  int max_suicides = 0;
  int min_suicides = -1;
  double total_suicides = 0;
  int temp_suicides = 0;
  int max_migrations = 0;
  int min_migrations = -1;
  double total_migrations = 0;
  int temp_migrations = 0;
  int deme_count = 0;


  for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);

    temp_points = deme.GetNumberOfPoints();
    temp_suicides = deme.GetSuicides();
    temp_migrations = deme.GetMigrationsOut();


    // Calculate Min
    if ((min_points == -1) || (temp_points < min_points)) {
      min_points = temp_points;
    }
    if ((min_suicides == -1) || (temp_suicides < min_suicides)) {
      min_suicides = temp_suicides;
    }
    if ((min_migrations == -1) || (temp_migrations < min_migrations)) {
      min_migrations = temp_migrations;
    }

    // Calculate Max
    if (temp_points > max_points) max_points = temp_points;
    if (temp_suicides > max_suicides) max_suicides = temp_suicides;
    if (temp_migrations > max_migrations) max_migrations = temp_migrations;

    total_points += temp_points;
    total_suicides += temp_suicides;
    total_migrations += temp_migrations;

    if (temp_points > 0) deme_count++;
  }

  df.Write((total_points/m_world->GetPopulation().GetNumDemes()), "AveragePoints[avpoints]" );
  df.Write(min_points, "MinPoints[minpoints]" );
  df.Write(max_points, "MaxPoints[maxpoints]" );
  df.Write(deme_count, "DemesWithPoints[demeswithpoints]");
  df.Write((total_suicides/m_world->GetPopulation().GetNumDemes()), "AverageSuicides[avsuicides]" );
  df.Write(min_suicides, "MinSuicides[minsuicides]" );
  df.Write(max_suicides, "MaxSuicides[maxsuicides]" );
  df.Write((total_migrations/m_world->GetPopulation().GetNumDemes()), "AverageMigrations[avmigrations]" );
  df.Write(min_migrations, "MinMigrations[minmigrations]" );
  df.Write(max_migrations, "MaxMigrations[maxmigrations]" );
  df.Write((total_suicides/total_migrations), "SuicideMigrationRate[suicidemigrationrate]" );

  df.Endl();
}


void cStats::CompeteDemes(const std::vector<double>& fitness)
{
  m_deme_fitness = fitness;
}


void cStats::PrintDemeCompetitionData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida compete demes data");
  df.WriteTimeStamp();
  df.Write(m_update, "Update [update]");

  double avg = std::accumulate(m_deme_fitness.begin(), m_deme_fitness.end(), 0.0);
  if(avg > 0.0) {
    avg /= m_deme_fitness.size();
  }
  df.Write(avg, "Avg. deme fitness [avgfit]");
  if(m_deme_fitness.size() > 0) {
    df.Write(*std::max_element(m_deme_fitness.begin(), m_deme_fitness.end()), "Max. deme fitness [maxfit]");
  } else {
    df.Write(0.0, "Max. deme fitness [maxfit]");
  }
  df.Endl();

  m_deme_fitness.clear();
}


/*! Prints the cell data from every cell, including the deme for that cell. */
void cStats::PrintCellData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Cell data per udpate.");
  df.WriteTimeStamp();

  for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    const cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
    df.Write(GetUpdate(), "Update [update]");
    df.Write(cell.GetID(), "Global cell ID [globalid]");
    df.Write(cell.GetDemeID(), "Deme ID for cell [demeid]");
    df.Write(cell.GetCellData(), "Cell data [data]");
    df.Endl();
  }
}


void cStats::PrintCurrentOpinions(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Current opinions of each organism.");
  df.WriteTimeStamp();
  df.WriteComment("1: Update [update]");
  df.WriteComment("2: Global cell ID [globalid]");
  df.WriteComment("3: Current opinion [opinion]");
  df.WriteComment("4: Cell ID of opinion [cellid]");
  df.FlushComments();

  // Build the cell id map:
  std::map<int,int> data_id_map;
  for (int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    const cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
    data_id_map[cell.GetCellData()] = cell.GetID();
  }

  for (int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    const cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
    df.WriteAnonymous(GetUpdate());
    df.WriteAnonymous(cell.GetID());
    if (cell.IsOccupied() && cell.GetOrganism()->HasOpinion()) {
      int opinion = cell.GetOrganism()->GetOpinion().first;
      df.WriteAnonymous(opinion);
      if (data_id_map.find(opinion) != data_id_map.end()) {
        df.WriteAnonymous(data_id_map[opinion]);
      } else {
        df.WriteAnonymous(-1);
      }
    } else {
      df.WriteAnonymous(0);
      df.WriteAnonymous(-1);
    }
    df.Endl();
  }
}


void cStats::PrintOpinionsSetPerDeme(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("Current fractions of opinions set in deme.");
	df.WriteComment("This files shows data for both treatable and untreatable demes.");
	df.WriteTimeStamp();

	cIntSum    treatableOpinionCounts, untreatableOpinionCounts;
	cDoubleSum treatableDensityCounts, untreatableDensityCounts;
	treatableOpinionCounts.Clear();
	untreatableOpinionCounts.Clear();
	treatableDensityCounts.Clear();
	untreatableDensityCounts.Clear();

	for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
    cDeme& deme = m_world->GetPopulation().GetDeme(i);
		int demeSize = deme.GetSize();
		if(deme.isTreatable()) {
			// accumultate counts for treatable demes
			for(int orgID = 0; orgID < demeSize; ++orgID) {
				treatableOpinionCounts.Add(deme.GetNumOrgsWithOpinion());
				treatableDensityCounts.Add(deme.GetDensity());
			}
		} else {
			// accumultate counts for untreatable demes
			for(int orgID = 0; orgID < demeSize; ++orgID) {
				untreatableOpinionCounts.Add(deme.GetNumOrgsWithOpinion());
				untreatableDensityCounts.Add(deme.GetDensity());
			}
		}
	}

	df.Write(GetUpdate(), "Update [update]");

	if(treatableOpinionCounts.N() > 0 && untreatableOpinionCounts.N() > 0) {
		df.Write(treatableOpinionCounts.Average(), "Average number of opinions set in Treatable demes");
		df.Write(untreatableOpinionCounts.Average(), "Average number of opinions set in Unreatable demes");
		df.Write(treatableDensityCounts.Average(), "Average density of Treatable demes");
		df.Write(untreatableDensityCounts.Average(), "Average density of Unreatable demes");
	} else {
		df.Write(untreatableOpinionCounts.Average(), "Average number of opinions set in demes");
		df.Write(untreatableDensityCounts.Average(), "Average density of demes");
	}
	df.Endl();
}

/*! Called when an organism issues a flash instruction.

 We do some pretty detailed tracking here in order to support the use of flash
 messages in deme competition.  All flashes are tracked per deme.

 Because we're tracking highly detailed information about flashes, if
 someone forgets to include the print event for synchronization, it's highly
 likely that Avida will run out of memory (not that this has happened *ahem*).
 So, the first time this method is called, we check to make sure that at least one
 of the print events is also called, otherwise we throw an error.
 */
void cStats::SentFlash(cOrganism& organism) {
	static bool event_checked=false;
	if(!event_checked && (m_world->GetEventsList() != 0)) {
		if(!m_world->GetEventsList()->IsEventUpcoming("PrintSynchronizationData")
			 && !m_world->GetEventsList()->IsEventUpcoming("PrintDetailedSynchronizationData")) {
			m_world->GetDriver().RaiseFatalException(-1, "When using the flash instruction, either the PrintSynchronizationData or PrintDetailedSynchronizationData events must also be used.");
		}
		event_checked = true;
	}

  ++m_flash_count;
	if(organism.GetOrgInterface().GetDeme() != 0) {
		const cDeme* deme = organism.GetOrgInterface().GetDeme();
		m_flash_times[GetUpdate()][deme->GetID()].push_back(deme->GetRelativeCellID(organism.GetCellID()));
	}
}


/*! Print statistics about synchronization flashes. */
void cStats::PrintSynchronizationData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida synchronization data");
  df.WriteTimeStamp();
  df.Write(m_update, "Update [update]");
  df.Write(m_flash_count, "Flash count [fcount]");
  df.Endl();

  m_flash_count = 0;
	m_flash_times.clear();
}


/*! Print detailed synchronization data. */
void cStats::PrintDetailedSynchronizationData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Detailed Avida synchronization data");
  df.WriteComment("Rows are (update, demeid, cellid) tuples, representing the update at which that cell flashed.");
  df.WriteTimeStamp();

	for(PopulationFlashes::iterator i=m_flash_times.begin(); i!=m_flash_times.end(); ++i) {
		for(DemeFlashes::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
			for(CellFlashes::iterator k=j->second.begin(); k!=j->second.end(); ++k) {
				df.Write(i->first, "Update [update]");
				df.Write(j->first, "Deme ID [demeid]");
				df.Write(*k, "Deme-relative cell ID that issued a flash at this update [relcellid]");
				df.Endl();
			}
		}
	}

	m_flash_times.clear();
}


/*! Called when a deme reaches consensus. */
void cStats::ConsensusReached(const cDeme& deme, cOrganism::Opinion consensus, int cellid) {
	m_consensi.push_back(ConsensusRecord(GetUpdate(), deme.GetID(), consensus, cellid));
}


/*! Print "simple" consensus information. */
void cStats::PrintSimpleConsensusData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida consensus data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
  df.Write((double)m_consensi.size(), "Consensus count [count]");
  df.Endl();
	m_consensi.clear();
}


/*! Print information about demes that have reached consensus. */
void cStats::PrintConsensusData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida consensus data");
  df.WriteTimeStamp();
	df.WriteColumnDesc("Update [update]");
	df.WriteColumnDesc("Deme ID [demeid]");
	df.WriteColumnDesc("Consensus value [consensus]");
	df.WriteColumnDesc("Cell ID [cellid]");
	df.FlushComments();

	for(Consensi::iterator i=m_consensi.begin(); i!=m_consensi.end(); ++i) {
		df.Write(i->update, "Update [update]");
		df.Write(i->deme_id, "Deme ID [demeid]");
		df.Write(i->consensus, "Consensus value [consensus]");
		df.Write(i->cell_id, "Cell ID [cellid]");
		df.Endl();
	}
	m_consensi.clear();
}


void cStats::PrintNumOrgsKilledData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Organisms killed using kill actions");
  df.WriteTimeStamp();
  df.WriteComment("First column is the current update and the second column lists the number of organisms killed");

  df.Write(m_update,   "Update");
  df.Write(sum_orgs_killed.Average(), "Avg Num Orgs Killed");
  df.Write(sum_unoccupied_cell_kill_attempts.Average(), "Avg Num Unoccupied Cell Kill Attempts");
  df.Write(sum_cells_scanned_at_kill.Average(), "Avg Num Cells Scanned By Kill Event");
  df.Endl();

  sum_orgs_killed.Clear();
  sum_unoccupied_cell_kill_attempts.Clear();
  sum_cells_scanned_at_kill.Clear();
} //End PrintNumOrgsKilledData()

void cStats::PrintMigrationData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Number of migrations made using the migratedemes event");
  df.WriteTimeStamp();
  df.WriteComment("First column is the current update and the second column lists the number of migrations made");

  df.Write(m_update,   "Update");
  df.Write(num_migrations, "Num Migrations");
  df.Endl();
} //End PrintMigrationData()


/* Print information pertinent to direct reciprocity experiments*/
void cStats::PrintDirectReciprocityData(const cString& filename){
	cDataFile& df = m_world->GetDataFile(filename);

	cDoubleSum donations;
	cDoubleSum reciprocations;
	cDoubleSum donors;
	cDoubleSum num_donations_received;

	cOrganism* org;

	int num_alt =0;
	int num_coop = 0;
	int num_lin_2 = 0;
	int num_lin_1 = 0;
	int total_org = 0;


	df.WriteComment("Avida organism direct reciprocity information");
	df.WriteTimeStamp();
	df.Write(m_update,   "Update [update]");


  for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		org = cell.GetOrganism();

    if(cell.IsOccupied()) {
			donations.Add(org->GetNumberOfDonations());
			num_donations_received.Add(org->GetNumberOfDonationsReceived());
			reciprocations.Add(org->GetNumberOfReciprocations());
			donors.Add(org->GetNumberOfDonors());
			if (org->GetNumberOfDonations() > 0) num_alt++;
			if ((org->GetNumberOfDonationsReceived() && org->GetNumberOfDonations()) > 0) num_coop++;
			if (org->GetLineageLabel() == 1) num_lin_1++;
			if (org->GetLineageLabel() == 2) num_lin_2++;
			total_org++;
	  }
	}

	df.Write(donations.Average(), "Avg. donations [donation]");
	df.Write(num_donations_received.Average(), "Avg. donations received [received]");
	df.Write(donors.Average(), "Avg. number of donor partners [partners]");
	df.Write(num_alt, "Number of altruists [altruists]");
	df.Write(num_coop, "Number of cooperators [cooperators]");
	df.Write(num_lin_1, "Number of organisms of lineage 1 [lineage1]");
	df.Write(num_lin_2, "Number of organisms of lineage 2 [lineage2]");
	df.Write(total_org, "Number of organisms in population [popsize]");

  df.Endl();


}


/* Print information about the string matching... */
void cStats::PrintStringMatchData(const cString& filename){
	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("Avida organism string donation information");
	df.WriteTimeStamp();
	df.Write(m_update,   "Update [update]");
	cOrganism* org;


	/*
	 // Interate through map of information.
	 map<cString,cDoubleSum>::iterator iter2;
	 for(iter2 = m_string_bits_matched.begin(); iter2 != m_string_bits_matched.end(); iter2++ ) {
	 df.Write(iter2->second.Average(), iter2->first);
	 }


	 // Create a map of the current tags in the population .
	 for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
	 cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
	 org = cell.GetOrganism();

	 if(cell.IsOccupied()) {
	 // Get tag and increment number of orgs.
	 m_tags[org->GetTagLabel()]++;
	 }
	 }

	 // print the tags
	 map<int, int>::iterator iter;
	 stringstream ss;
	 for(iter = m_tags.begin(); iter != m_tags.end(); iter++ ) {
	 ss << iter->first;
	 string name = ss.str();
	 df.Write(iter->second, name.c_str());
	 iter->second = 0;
	 }*/


	// Print data about strings:
	std::map <int, cDoubleSum> m_strings_stored;
	std::map <int, cDoubleSum> m_strings_produced;
	cDoubleSum total;
	int min = -1;
	int onhand = 0;
	int instant_perfect_match = 0;
	int instant_perfect_match_org = 0;
	int nothing  =0;
	int specialists = 0;
	int generalists = 0;
	int type_prod = 0;

	// Get the number of strings
	int num = m_world->GetEnvironment().GetNumberOfMatchStrings();
	for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		org = cell.GetOrganism();
		min = -1;
		onhand = 0;
		type_prod = 0;

    if(cell.IsOccupied()) {
			for (int j = 0; j<num; j++) {
				onhand = org->GetNumberStringsOnHand(j);
				if ((min == -1) || (onhand < min)){
					min = onhand;
				}
				m_strings_stored[j].Add(onhand);
				total.Add(onhand);
				m_strings_produced[j].Add(org->GetNumberStringsProduced(j));

				if (org->GetNumberStringsProduced(j)) type_prod++;

			}

			instant_perfect_match += min;
			if (min > 0) instant_perfect_match_org++;
			if (type_prod ==0) nothing++;
			if (type_prod == 1) specialists++;
			if (type_prod > 1) generalists++;
		}

	}

	// print the string info
	for (int k=0; k<num; k++) {
		string name = m_world->GetEnvironment().GetMatchString(k).GetData();
		name = "produced" + name;
		df.Write(m_strings_produced[k].Average(), name.c_str());

		name = m_world->GetEnvironment().GetMatchString(k).GetData();
		name = "stored" + name;
		df.Write(m_strings_stored[k].Average(), name.c_str());

	}
	df.Write(total.Average(), "totalStoredAverage");

	// Print number of perfect matches
	df.Write(m_perfect_match.Sum(), "PerfectMatchStringElapse[ps]");
	m_perfect_match.Clear();
	// Print number of perfect matches
	df.Write(m_perfect_match_org.Sum(), "PerfectMatchOrgElapse[pso]");
	m_perfect_match_org.Clear();
	df.Write(instant_perfect_match, "PerfectMatchStringInstant[psi]");
	// Print number of perfect matches
	df.Write(instant_perfect_match_org, "PerfectMatchOrgInstant[psoi]");
	df.Write(nothing, "Producednothing[nothing]");
	df.Write(generalists, "Generalists[generalists]");
	df.Write(specialists, "Specialists[specialists]");


  df.Endl();
}

/* Print information about the reputation... */
void cStats::PrintReputationData(const cString& filename){
	cDataFile& df = m_world->GetDataFile(filename);

	cDoubleSum reputations;
	cDoubleSum donations;
	cDoubleSum reciprocations;
	cDoubleSum donors;
	cDoubleSum k;
	cDoubleSum num_donations_received;
	cDoubleSum amount_donations_received;
	cDoubleSum num_failed_reputation_inc;
	cDoubleSum own_raw_mat;
	cDoubleSum other_raw_mat;


	// difference between how many an organism donated & how many it received
	cDoubleSum disparity;

	cOrganism* org;
	int min_rep = 100;
	int max_rep = 0;
	int cur_rep;
	int num_alt =0;
	int num_coop = 0;



	df.WriteComment("Avida organism reputation information -- average donations, min donations, max donations");
	df.WriteTimeStamp();
	df.Write(m_update,   "Update [update]");


  for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		org = cell.GetOrganism();

    if(cell.IsOccupied()) {
			cur_rep = org->GetReputation();

			if (cur_rep < min_rep) min_rep = cur_rep;
			if (max_rep < cur_rep) max_rep = cur_rep;
			reputations.Add(cur_rep);
			donations.Add(org->GetNumberOfDonations());
			num_donations_received.Add(org->GetNumberOfDonationsReceived());
			amount_donations_received.Add(org->GetAmountOfDonationsReceived());
			own_raw_mat.Add(org->GetSelfRawMaterials());
			other_raw_mat.Add(org->GetOtherRawMaterials());

			reciprocations.Add(org->GetNumberOfReciprocations());
			donors.Add(org->GetNumberOfDonors());
			num_failed_reputation_inc.Add(org->GetFailedReputationIncreases());
//			k.Add(org->GetK());

			disparity.Add(org->GetNumberOfDonations() - org->GetOtherRawMaterials());

			if (org->GetNumberOfDonations() > 0) num_alt++;
			if ((org->GetNumberOfDonationsReceived() && org->GetNumberOfDonations()) > 0) num_coop++;


	  }
	}
	df.Write(reputations.Average(), "Avg. reputation [reputation]");
	//	df.Write(reputations.StdDeviation(), "Standard Deviation [repstddev]");
	//	df.Write(min_rep, "Minimum reputation");
	//	df.Write(max_rep, "Maximum reputation");
	df.Write(donations.Average(), "Avg. donations [donation]");
	//	df.Write(num_donations_received.Average(), "Avg. donations received [received]");
	//	df.Write(amount_donations_received.Average(), "Avg. number donations received [amount]");
	//	df.Write(reciprocations.Average(), "Avg. reciprocations [reciprocation]");
	//	df.Write(disparity.Average(), "Disparity between donations and collections [disparity]");
	df.Write(donors.Average(), "Avg. number of donor partners [partners]");
	//	df.Write(num_failed_reputation_inc.Average(), "Avg. number of reputation increase failures [failure]");
	//	df.Write(recip_prob_change.Average(), "Avg. change in reciprocation probability [recipprob]");

	df.Write(num_alt, "Number of altruists [altruists]");
	df.Write(num_coop, "Number of cooperators [cooperators]");
	df.Write(own_raw_mat.Average(), "Avg. own raw mat [ownrawmat]");
	df.Write(other_raw_mat.Average(), "Avg. other raw mat [otherrawmat]");
	//	df.Write(num_all_strings, "Number of orgs with all strings [allstrings]");

	//	df.Write(k.Average(), "Avg. k of organisms [k]");
	//	df.Write(m_donate_to_donor, "Number of donate to donor [donatedonor]");
	//	df.Write(m_donate_to_facing, "Number of donate to facing [donatefacing]");



  df.Endl();
}

/*
  Cycle through the population -- count the number of altruists in each bin.
  Also average their shaded donations.
  Check how many prefer the shaded strategy

 */
void cStats::PrintShadedAltruists(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("The number of organisms in different bins of shaded altruism");

	// Cycle through the population -- count the number of altruists in each bin.
	// Also average their shaded donations.
	// Check how many prefer the shaded strategy

	//int num_shaded_pref = 0; //!num orgs that prefer shaded
	int pop = m_world->GetPopulation().GetSize(); //!the population size for convenience
	int shaded_100 = 0;
	int shaded_90 = 0;
	int shaded_80 = 0;
	int shaded_70 = 0;
	int shaded_60 = 0;
	int shaded_50 = 0;
	int shaded_40 = 0;
	int shaded_30 = 0;
	int shaded_20 = 0;
	int shaded_10 = 0;
	int shaded_0 = 0;
	int total_shaded = 0;

	//int other_donations = 0;
	int shade_of_gb;
	cOrganism* org;


	for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
		shade_of_gb = 0;
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		org = cell.GetOrganism();

    if(cell.IsOccupied()) {
			org = cell.GetOrganism();

			const cInstSet& inst_set = m_world->GetHardwareManager().GetDefaultInstSet();
			const int num_inst = inst_set.GetSize();
			for (int i = 0; i < num_inst; i++) {
				if ((inst_set.GetName(i) == "donate-shadedgb") && (org->GetPhenotype().GetTestCPUInstCount().GetSize() > 0)) {
					shade_of_gb = org->GetPhenotype().GetTestCPUInstCount()[i];
				}
			}
			if (shade_of_gb == 100) shaded_100++;
			if (shade_of_gb > 90) shaded_90++;
			if (shade_of_gb > 80) shaded_80++;
			if (shade_of_gb > 70) shaded_70++;
			if (shade_of_gb > 60) shaded_60++;
			if (shade_of_gb > 50)	shaded_50++;
			if (shade_of_gb > 40)	shaded_40++;
			if (shade_of_gb > 30)	shaded_30++;
			if (shade_of_gb > 20)	shaded_20++;
			if (shade_of_gb > 10)	shaded_10++;
			if (shade_of_gb > 0) shaded_0++;
			total_shaded += shade_of_gb;
		}
	}

	float high_alt = (float) shaded_90/pop;
	float avg_shade = (float) total_shaded/pop;

	df.WriteComment("Bins of orgs of shaded strategies.");
	df.WriteTimeStamp();
	df.Write(m_update,   "Update [update]");
	df.Write(pop, "Population [population]");
	df.Write(shaded_100, "shaded-100 [shaded100]");
	df.Write(shaded_90, "shaded-90 [shaded90]");
	df.Write(shaded_80, "shaded-80 [shaded80]");
	df.Write(shaded_70, "shaded-70 [shaded70]");
	df.Write(shaded_60, "shaded-60 [shaded60]");
	df.Write(shaded_50, "shaded-50 [shaded50]");
	df.Write(shaded_40, "shaded-40 [shaded40]");
	df.Write(shaded_30, "shaded-30 [shaded30]");
	df.Write(shaded_20, "shaded-20 [shaded20]");
	df.Write(shaded_10, "shaded-10 [shaded10]");
	df.Write(shaded_0, "shaded-0 [shaded0]");
	df.Write(high_alt, "percent-high-alt  [highalt]");
	df.Write(avg_shade, "avg-shade [avgshade]");
	df.Endl();

}

/*
 Print data regarding group formation.
 */
void cStats::PrintGroupsFormedData(const cString& filename)
{

	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("Information about the groups joined and used by the organisms");

	map<int,int> groups = m_world->GetPopulation().GetFormedGroups();

	map <int,int>::iterator itr;
	double avg_size = 0.0;
	double avg_size_wout_empty = 0.0;
	double max_size = 0.0;
	double min_size = 100000000000.0;
	double active_groups = 0.0;
	double groups_per_org = 0.0;

	for(itr = groups.begin();itr!=groups.end();itr++) {
		double cur_size = itr->second;
		avg_size += cur_size;
		if (cur_size > max_size) max_size = cur_size;
		if (cur_size < min_size) min_size = cur_size;
		if (cur_size > 0) {
			active_groups++;
			avg_size_wout_empty += cur_size;
		}
	}

	cOrganism* org;
	for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		org = cell.GetOrganism();

    if(cell.IsOccupied()) {
			org = cell.GetOrganism();
			groups_per_org += org->HasOpinion();
		}
	}

	avg_size = avg_size / groups.size();
	avg_size_wout_empty = avg_size_wout_empty / active_groups;
	groups_per_org = groups_per_org / m_world->GetPopulation().GetSize();
	df.WriteTimeStamp();
	df.Write(m_update,   "Update [update]");
	df.Write((double)groups.size(), "number of groups [num]");
	df.Write(avg_size, "average size of groups [avgsize]");
	df.Write(avg_size_wout_empty, "average size of  non-emptygroups [avgsizene]");
	df.Write(max_size, "max size of groups [maxsize]");
	df.Write(min_size, "min size of groups [minsize]");
	df.Write(active_groups, "active groups [actgroup]");
	df.Write(groups_per_org, "groups per org life [groupsperorg]");


	df.Endl();

}

/*
 Print data regarding the ids of used groups.
 */
void cStats::PrintGroupIds(const cString& filename)
{

	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("The ids of groups used.");

	map<int,int> groups = m_world->GetPopulation().GetFormedGroups();

	map <int,int>::iterator itr;

	df.WriteTimeStamp();

	for(itr = groups.begin();itr!=groups.end();itr++) {
		double cur_size = itr->second;
/*		if (cur_size > 0)*/ {
			df.Write(m_update,   "Update [update]");
			df.Write(itr->first, "group id [groupid]");
			df.Write(cur_size, "size of groups [grsize]");
			df.Endl();
		}
	}
	df.Endl();
}

// Print data for each group's tolerances. 
void cStats::PrintGroupTolerance(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Group level tolerance data.");
  df.WriteTimeStamp();

  map<int, int> groups = m_world->GetPopulation().GetFormedGroups();
  map<int, int>::iterator itr;

  for(itr = groups.begin(); itr != groups.end(); itr++) {
    double cur_size = itr->second;
    int i = itr->first;
    df.Write(m_update,                                                  "Update [update]");
    df.Write(itr->first,                                                "group id [groupid]");
    df.Write(cur_size,                                                  "size of groups [grsize]");
    df.Write(resource_count[i],"group resource available [grfood]");
    df.Write(resource_count[i] / cur_size, "per capita group resource available [grfoodper]");
    if (m_world->GetConfig().TOLERANCE_WINDOW.Get() > 0) {
      df.Write(m_world->GetPopulation().CalcGroupOddsImmigrants(i, -1),   "odds for immigrants coming into group [oddsimmigrants]");
      df.Write(m_world->GetPopulation().CalcGroupAveImmigrants(i, -1),    "average intra-group tolerance to immigrants [aveimmigrants]");
      df.Write(m_world->GetPopulation().CalcGroupSDevImmigrants(i, -1),   "standard deviation for group tolerance to immigrants [sdevimmigrants]");
      df.Write(m_world->GetPopulation().CalcGroupOddsOffspring(i),    "odds for offspring being accepted by group [oddsoffspring]");
      df.Write(m_world->GetPopulation().CalcGroupAveOthers(i),        "average intra-group tolerance to other offspring being born into group [aveothers]");
      df.Write(m_world->GetPopulation().CalcGroupSDevOthers(i),       "standard deviation for group tolerance to other offspring being born into the group [sdevothers]");
      df.Write(m_world->GetPopulation().CalcGroupAveOwn(i),           "average intra-group tolerance to individual's own offspring [aveown]");
      df.Write(m_world->GetPopulation().CalcGroupSDevOwn(i),          "standard deviation for tolerance to own offspring [sdevown]");
    }
    df.Endl();
  }
}

void cStats::PrintGroupMTTolerance(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Group level tolerance data by mating type.");
  df.WriteTimeStamp();
  
  map<int, int> groups = m_world->GetPopulation().GetFormedGroups();
  map<int, int>::iterator itr;
  
  for(itr = groups.begin(); itr != groups.end(); itr++) {
    double cur_size = itr->second;
    int i = itr->first;
    df.Write(m_update,                                                  "Update");
    df.Write(itr->first,                                                "group id");
    df.Write(cur_size,                                                  "group size");
    df.Write(m_world->GetPopulation().NumberGroupFemales(i),            "number group females");
    df.Write(m_world->GetPopulation().NumberGroupMales(i),              "number group males");
    df.Write(m_world->GetPopulation().NumberGroupJuvs(i),               "number group juvs");
    if (m_world->GetConfig().TOLERANCE_WINDOW.Get() > 0) {
      df.Write(m_world->GetPopulation().CalcGroupOddsImmigrants(i, 0),   "immigrant female odds");
      df.Write(m_world->GetPopulation().CalcGroupAveImmigrants(i, 0),    "ave female-female tolerance");
      df.Write(m_world->GetPopulation().CalcGroupSDevImmigrants(i, 0),   "sd female-female tolerance");
      df.Write(m_world->GetPopulation().CalcGroupOddsImmigrants(i, 1),   "immigrant male odds");
      df.Write(m_world->GetPopulation().CalcGroupAveImmigrants(i, 1),    "ave male-male tolerance");
      df.Write(m_world->GetPopulation().CalcGroupSDevImmigrants(i, 1),   "sd male-male tolerance");
      df.Write(m_world->GetPopulation().CalcGroupOddsImmigrants(i, 2),   "immigrant juv odds");
      df.Write(m_world->GetPopulation().CalcGroupAveImmigrants(i, 2),    "ave juv-juv tolerance");
      df.Write(m_world->GetPopulation().CalcGroupSDevImmigrants(i, 2),   "sd juv-juv tolerance");
    }
    df.Write(resource_count[i],                                         "group resource available");
    df.Write(resource_count[i] / cur_size,                              "per capita group resource available");
    df.Endl();
  }
}

// Prints number of executions within the update of tolerance instructions executed,
// differentiated between different nop-modifications on each. 
void cStats::PrintToleranceInstructionData(const cString& filename)
{
  const int num_tol_inst = 8;
  tArray<cString> m_is_tolerance_inst_names(num_tol_inst);
  m_is_tolerance_inst_names[0] = "inc-tolerance_Immigrants";
  m_is_tolerance_inst_names[1] = "inc-tolerance_OffspringOwn";
  m_is_tolerance_inst_names[2] = "inc-tolerance_OffspringOthers";
  m_is_tolerance_inst_names[3] = "dec-tolerance_Immigrants";
  m_is_tolerance_inst_names[4] = "dec-tolerance_OffspringOwn";
  m_is_tolerance_inst_names[5] = "dec-tolerance_OffspringOthers";
  m_is_tolerance_inst_names[6] = "get-tolerance";
  m_is_tolerance_inst_names[7] = "get-group-tolerance";

  if (m_is_tolerance_exe_counts.GetSize() != num_tol_inst) m_is_tolerance_exe_counts.Resize(num_tol_inst);

  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida tolerance instruction executions per update");
  df.WriteTimeStamp();

  df.Write(m_update, "Update");

  for (int i = 0; i < num_tol_inst; i++) {
    df. Write((m_is_tolerance_exe_counts[i].first == m_update) ? m_is_tolerance_exe_counts[i].second : 0 , m_is_tolerance_inst_names[i]);
  }

  df.Endl();
}

// Prints the circumstances around each tolerance instruction executed within the last update. 
void cStats::PrintToleranceData(const cString& filename)
{
  // TRACK_TOLERANCE must be on in config for output file to function
  if(!m_world->GetConfig().TRACK_TOLERANCE.Get()) {
    m_world->GetDriver().RaiseFatalException(-1, "TRACK_TOLERANCE option must be turned on in avida.cfg for PrintToleranceData to function.");
  }

  const int num_tol_inst = 8;
  tArray<cString> m_is_tolerance_inst_names(num_tol_inst);
  m_is_tolerance_inst_names[0] = "inc-tolerance_Immigrants";
  m_is_tolerance_inst_names[1] = "inc-tolerance_OffspringOwn";
  m_is_tolerance_inst_names[2] = "inc-tolerance_OffspringOthers";
  m_is_tolerance_inst_names[3] = "dec-tolerance_Immigrants";
  m_is_tolerance_inst_names[4] = "dec-tolerance_OffspringOwn";
  m_is_tolerance_inst_names[5] = "dec-tolerance_OffspringOthers";
  m_is_tolerance_inst_names[6] = "get-tolerance";
  m_is_tolerance_inst_names[7] = "get-group-tolerance";

  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida circumstance data for each tolerance instruction pre-execution");
  df.WriteTimeStamp();

  for (int n = 0; n < m_is_tolerance_exe_insts.GetSize(); n++) {
    if (m_is_tolerance_exe_insts[n].update == m_update) {
      df.Write(m_is_tolerance_exe_insts[n].update, "Update [update]");
      df.Write(m_is_tolerance_inst_names[m_is_tolerance_exe_insts[n].inst], "Tolerance instruction [inst]");
      df.Write(m_is_tolerance_exe_insts[n].gr_id, "group id [groupid]");
      df.Write(m_is_tolerance_exe_insts[n].gr_size, "size of group [grsize]");
      df.Write(m_is_tolerance_exe_insts[n].res_level, "group resource available [grfood]");
      df.Write(m_is_tolerance_exe_insts[n].odds_immigrants, "odds for immigrants coming into the group [oddsimmigrants]");
      df.Write(m_is_tolerance_exe_insts[n].odds_offspring_own, "odds for org's own offspring to stay in group [oddsown]");
      df.Write(m_is_tolerance_exe_insts[n].odds_offspring_others, "odds for offspring in group [oddsothers]");
      df.Write(m_is_tolerance_exe_insts[n].tol_immigrants, "org's tolerance for immigrants [tol-immi]");
      df.Write(m_is_tolerance_exe_insts[n].tol_own, "org's tolerance for own offspring [tol-own]");
      df.Write(m_is_tolerance_exe_insts[n].tol_others, "org's tolerance for other offspring in the group [tol-others]");
      df.Write(m_is_tolerance_exe_insts[n].tol_max, "tolerance max [tol-max]");
      df.Endl();
    }
  }
}

void cStats::PushToleranceInstExe(int tol_inst)
{
  const int num_tol_inst = 8;
  if (m_is_tolerance_exe_counts.GetSize() != num_tol_inst) m_is_tolerance_exe_counts.Resize(num_tol_inst);

  if (m_is_tolerance_exe_counts[tol_inst].first == m_update) {
    m_is_tolerance_exe_counts[tol_inst].second++;
  } else {
    m_is_tolerance_exe_counts[tol_inst].first = m_update;
    m_is_tolerance_exe_counts[tol_inst].second = 1;
  }
}

// Adds a record of a tolerance instruction execution w its circumstances. 
void cStats::PushToleranceInstExe(int tol_inst, int group_id, int group_size, double resource_level, double odds_immi,
          double odds_own, double odds_others, int tol_immi, int tol_own, int tol_others, int tol_max)
{
  const int num_tol_inst = 8;
  if (m_is_tolerance_exe_counts.GetSize() != num_tol_inst) m_is_tolerance_exe_counts.Resize(num_tol_inst);

  if (m_is_tolerance_exe_insts.GetSize() > 0) {
    if (m_is_tolerance_exe_insts[0].update != m_update) {
      m_is_tolerance_exe_insts.ResizeClear(0);
    }
  }

  if (m_is_tolerance_exe_counts[tol_inst].first == m_update) {
    m_is_tolerance_exe_counts[tol_inst].second++;
  } else {
    m_is_tolerance_exe_counts[tol_inst].first = m_update;
    m_is_tolerance_exe_counts[tol_inst].second = 1;
  }

  s_inst_circumstances tol_circ;
  tol_circ.update = m_update;
  tol_circ.inst = tol_inst;
  tol_circ.gr_id = group_id;
  tol_circ.gr_size = group_size;
  tol_circ.res_level = resource_level;
  tol_circ.odds_immigrants = odds_immi;
  tol_circ.odds_offspring_own = odds_own;
  tol_circ.odds_offspring_others = odds_others;
  tol_circ.tol_immigrants = tol_immi;
  tol_circ.tol_own = tol_own;
  tol_circ.tol_others = tol_others;
  tol_circ.tol_max = tol_max;

  m_is_tolerance_exe_insts.Push(tol_circ);
}

// Clears all tolerance execution circumstances. 
void cStats::ZeroToleranceInst()
{
  const int num_tol_inst = 8;
  for (int i = 0; i < num_tol_inst; i++) {
    m_is_tolerance_exe_counts[i] = make_pair(-1,-1);
  }
  m_is_tolerance_exe_insts.ResizeClear(0);
}
/*
 data about donate specific push: id, donated id, kin,
 */

void cStats::PushDonateSpecificInstExe(int org_id, int cell_id, int recipient_id, int recipient_cell_id, int relatedness, int recip_is_beggar, int num_donates)
{
  if (m_donate_specific.GetSize() > 0) {
    if (m_donate_specific[0].update != m_update) {
      m_donate_specific.ResizeClear(0);
    }
  }
  
  sDonateSpecificCircumstances donates;
  donates.update = GetUpdate();
  donates.org_id = org_id;
  donates.cell_id = cell_id;
  donates.recipient_id = recipient_id;
  donates.recipient_cell_id = recipient_cell_id;
  donates.relatedness = relatedness;
  donates.recip_is_beggar = recip_is_beggar;
  donates.num_donates = num_donates;
  
  m_donate_specific.Push(donates);
}

// Prints the circumstances around each tolerance instruction executed within the last update. 
void cStats::PrintDonateSpecificData(const cString& filename)
{
  // TRACK_TOLERANCE must be on in config for output file to function
  if(!m_world->GetConfig().TRACK_DONATES.Get()) {
    m_world->GetDriver().RaiseFatalException(-1, "TRACK_DONATIONS option must be turned on in avida.cfg for PrintDonateSpecificData to function.");
  }
  
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Avida circumstance data for each donate-specific instruction pre-execution");
  df.WriteTimeStamp();
  
  for (int i = 0; i < m_donate_specific.GetSize(); i++) {
    if (m_donate_specific[i].update == m_update) {
      df.Write(m_donate_specific[i].update, "Update [update]");
      df.Write(m_donate_specific[i].org_id, "id of donor [org_id]");
      df.Write(m_donate_specific[i].cell_id, "cell id of donor [cell_id]");
      df.Write(m_donate_specific[i].recipient_id, "id of recipient [recipient_id]");
      df.Write(m_donate_specific[i].recipient_cell_id, "cell id of teh recipient [recipient_cell_id]");
      df.Write(m_donate_specific[i].relatedness, "relatedness [relatedness]");
      df.Write(m_donate_specific[i].recip_is_beggar, "recip_is_beggar [is recipient beggar]");
      df.Write(m_donate_specific[i].num_donates, "num_donates [lifetime num donates]");
      df.Endl();
    }
  }
}
/*
 Print data regarding the living org targets.
 */
void cStats::PrintTargets(const cString& filename)
{
	cDataFile& df = m_world->GetDataFile(filename);
	df.WriteComment("Targets in use on update boundary.");
  df.WriteComment("-2: is predator, -1: no targets(default), >=0: id of environmental resource targeted).");
  df.WriteComment("Format is update + target0 + count0 + target1 + count1 ...");
	df.WriteTimeStamp();

  df.Write(m_update, "Update");
  
  bool has_pred = false;
  int offset = 1;
  if (m_world->GetConfig().PRED_PREY_SWITCH.Get() > -1) { 
    has_pred = true;
    offset = 2;
  }
    
  // ft's may not be sequentially numbered
  bool dec_prey = false;
  bool dec_pred = false;
  int num_targets = 0;
  std::set<int> fts_avail = m_world->GetEnvironment().GetTargetIDs();
  set <int>::iterator itr;    
  for (itr = fts_avail.begin();itr!=fts_avail.end();itr++) {
    num_targets++; 
    if (*itr == -1 && !dec_prey) { 
      offset--; 
      dec_prey = true; 
    }
    if (*itr == -2 && !dec_pred) {
      offset--;  
      dec_pred = true; 
    }
  }

  tArray<int> raw_target_list;
  raw_target_list.Resize(num_targets);
  raw_target_list.SetAll(0);
  int this_index = 0;
  for (itr = fts_avail.begin(); itr!=fts_avail.end(); itr++) {
    raw_target_list[this_index] = *itr; 
    this_index++;
  }
    
  tArray<int> target_list;
  int tot_targets = num_targets + offset;
  target_list.Resize(tot_targets);
  target_list.SetAll(0);

  target_list[0] = -1;
  if (has_pred) {
    target_list[0] = -2;
    target_list[1] = -1;
  }
      
  for (int i = 0; i < raw_target_list.GetSize(); i++) {
    if (raw_target_list[i] >= 0) target_list[i + offset] = raw_target_list[i];
  }
  
  tArray<int> org_targets;
  org_targets.Resize(tot_targets);
  org_targets.SetAll(0);
  
  const tSmartArray <cOrganism*> live_orgs = m_world->GetPopulation().GetLiveOrgList();
  for (int i = 0; i < live_orgs.GetSize(); i++) {  
    cOrganism* org = live_orgs[i];
    int this_target = org->GetForageTarget();

    int this_index = this_target;
    for (int i = 0; i < target_list.GetSize(); i++) {
      if (target_list[i] == this_target) {
        this_index = i;
        break;
      }
    }
    org_targets[this_index]++;
  }
  for (int target = 0; target < org_targets.GetSize(); target++) {
      df.Write(target_list[target], "Target ID");
      df.Write(org_targets[target], "Num Orgs Targeting ID");
  }
  df.Endl();
}

/*! Track named network stats.
 */
void cStats::NetworkTopology(const network_stats_t& ns) {
	for(network_stats_t::const_iterator i=ns.begin(); i!=ns.end(); ++i) {
		m_network_stats[i->first].Add(i->second);
	}
}


/*! Print and reset network statistics.
 */
void cStats::PrintDemeNetworkData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Deme network statistics");
  df.WriteTimeStamp();
	df.Write(GetUpdate(), "Update [update]");
	for(avg_network_stats_t::iterator i=m_network_stats.begin(); i!=m_network_stats.end(); ++i) {
		df.Write(i->second.Average(), i->first.c_str());
	}
	df.Endl();
	m_network_stats.clear();
}

void cStats::PrintDemeNetworkTopology(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Deme network topologies.");
  df.WriteTimeStamp();

	for(int i=0; i<m_world->GetPopulation().GetNumDemes(); ++i) {
		m_world->GetPopulation().GetDeme(i).GetNetwork().PrintTopology(df);
	}
}


/*! Called when an organism metabolizes a genome fragment.
 */
void cStats::GenomeFragmentMetabolized(cOrganism* organism, const Sequence& fragment) {
	m_hgt_metabolized.Add(fragment.GetSize());
}

/*! Called when a fragment is inserted into an offspring's genome via HGT.
 */
void cStats::GenomeFragmentInserted(cOrganism* organism, const Sequence& fragment, const cGenomeUtil::substring_match& location) {
	m_hgt_inserted.Add(fragment.GetSize());
}

/*!	Print HGT statistics.
 */
void cStats::PrintHGTData(const cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Horizontal gene transfer statistics");
  df.WriteTimeStamp();
	df.Write(GetUpdate(), "Update [update]");
	df.Write(m_hgt_metabolized.Count(), "Total count of metabolized genome fragments [metcount]");
	df.Write(m_hgt_metabolized.Sum(), "Total size of metabolized genome fragments [metsize]");
	df.Write(m_hgt_inserted.Count(), "Total count of insertion events [inscount]");
	df.Write(m_hgt_inserted.Sum(), "Total size of insertion events [inssize]");
	df.Endl();

	m_hgt_metabolized.Clear();
	m_hgt_inserted.Clear();
}


/*! Log a message.
 */
void cStats::LogMessage(const cOrgMessage& msg, bool dropped, bool lost) {
	m_message_log.push_back(message_log_entry_t(GetUpdate(),
      msg.GetSender()->GetDeme()->GetID(),
      msg.GetSenderCellID(),
      msg.GetReceiverCellID(),
      msg.GetTransCellID(),
      msg.GetData(),
      msg.GetLabel(),
      dropped,
      lost));
}

/*! Prints logged messages.
 */
void cStats::PrintMessageLog(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

	df.WriteComment("Log of all messages sent in population.");
  df.WriteTimeStamp();

	for(message_log_t::iterator i=m_message_log.begin(); i!=m_message_log.end(); ++i) {
		df.Write(i->update, "Update [update]");
		df.Write(i->deme, "Deme ID [deme]");
		df.Write(i->src_cell, "Source [src]");
		df.Write(i->dst_cell, "Destination [dst]");
        df.Write(i->transmit_cell, "Transmission_cell [trs]");
		df.Write(i->msg_data, "Message data [data]");
		df.Write(i->msg_label, "Message label [label]");
		df.Write(i->dropped, "Dropped [dropped]");
		df.Write(i->lost, "Lost [lost]");
		df.Endl();
	}

	m_message_log.clear();
}


/* Add that an organism performed a task at a certain age */
void cStats::AgeTaskEvent(int org_id, int task_id, int org_age) {
	reaction_age_map[task_id].Add(org_age);
}

/* Add the time between two tasks */ 
void cStats::AddTaskSwitchTime(int t1, int t2, int time) {
  intrinsic_task_switch_time[make_pair(t1, t2)].Add(time); 
}


/* Track the relationship between the age of the organism and the task that they perform */

void cStats::PrintIntrinsicTaskSwitchingCostData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);
	const cEnvironment& env = m_world->GetEnvironment();
  std::map<std::pair<int, int>, cDoubleSum>::iterator iter;
  
  df.WriteComment("Number of cyles it takes to change between tasks");
  df.WriteTimeStamp();
	df.WriteColumnDesc("Update [update]");
  df.WriteColumnDesc("Task 1 [t1]");
  df.WriteColumnDesc("Task 2 [t2]");
  df.WriteColumnDesc("Mean cycles [mc]");

  
  for (iter=intrinsic_task_switch_time.begin(); iter!=intrinsic_task_switch_time.end(); ++iter) {
    df.Write(m_update,   "Update [update]");
    df.Write(iter->first.first,   "Task 1 [t1]");
    df.Write(iter->first.second,   "Task 2 [t2]");
    df.Write(iter->second.Average(),   "Mean cycles [mc]");
    iter->second.Clear();
    df.Endl();
  }
  intrinsic_task_switch_time.clear();
}


/* Track the relationship between the age of the organism and the task that they perform */

void cStats::PrintAgePolyethismData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);
	const cEnvironment& env = m_world->GetEnvironment();
  const int num_tasks = env.GetNumTasks();

  df.WriteComment("Tasks, mean organism age, and variance of ages");
  df.WriteTimeStamp();
	df.WriteColumnDesc("Update [update]");
	for(int i = 0; i < num_tasks; i++) {
		string s;
		std::stringstream out;
		out << i;
		s = out.str();
		string av_comment = "Task " + s + " Organism Age Mean [meanorgage" + s + "]";
		string err_comment = "Task " + s + " Organism Age Standard Error [errorgage" + s + "]";
		df.WriteColumnDesc(av_comment.c_str());
		df.WriteColumnDesc(err_comment.c_str());

	}

	df.FlushComments();
	df.Write(m_update,   "Update");
	for(int i = 0; i < num_tasks; i++) {
		string s;
		std::stringstream out;
		out << i;
		s = out.str();

		string av_comment = "Task " + s + " Organism Age Mean [meanorgage" + s + "]";
		string err_comment = "Task " + s + " Organism Age Standard Error [errorgage" + s + "]";
		if (reaction_age_map[i].Count()  > 0) {
			df.Write(reaction_age_map[i].Average(), av_comment.c_str());
			df.Write(reaction_age_map[i].StdError(), err_comment.c_str());
		} else {
			df.Write(0, av_comment.c_str());
			df.Write(0, err_comment.c_str());
		}

		reaction_age_map[i].Clear();
	}
	df.Endl();
}


void cStats::PrintDenData(const cString& filename) {
  if (m_world->GetConfig().USE_AVATARS.Get() <= 0) return; 
  
  int juv_age = m_world->GetConfig().JUV_PERIOD.Get();
  
  const cResourceLib& resource_lib = m_world->GetEnvironment().GetResourceLib();
  
  int num_juvs = 0;
  int num_guards = 0;
  
  for (int i = 0; i < m_world->GetPopulation().GetSize(); i++) {
    cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
    
    if (!cell.HasAV()) continue;
    
    tArray<double> cell_res;
    cell_res = m_world->GetPopulation().GetCellResources(i, m_world->GetDefaultContext());
    
    for (int j = 0; j < cell_res.GetSize(); j++) {
      if (resource_lib.GetResource(j)->GetHabitat() == 4 && cell_res[j] > 0) {
        // for every x juvs, we require 1 adult...otherwise use killprob on the rest
        tArray<cOrganism*> cell_avs = cell.GetCellAVs();    // cell avs are already randomized

        for (int k = 0; k < cell_avs.GetSize(); k++) {
          if (cell_avs[k]->GetPhenotype().GetTimeUsed() < juv_age) { 
            num_juvs++;
          }
          else num_guards++;
        }
        
        break;  // only do this once if two dens overlap
      }
    }
  }

  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Number of juveniles and adults in dens");
  df.WriteTimeStamp();
	df.WriteColumnDesc("Update [update]");
  df.WriteColumnDesc("Juveniles [juveniles]");
	df.WriteColumnDesc("Adults [adults]");
  df.FlushComments();
	df.Write(m_update,   "Update");
  df.Write(num_juvs,   "Juveniles");
	df.Write(num_guards,   "Adults");
	df.Endl();

  
}




/*! Print statistics related to the diversity of reactions performed by a deme
 prior to replication.  */
void cStats::PrintDemeReactionDiversityReplicationData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida deme reaction diversity replication data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");

	while(m_switching.size()>100) {
		m_switching.pop_front();
	}
	while(m_deme_diversity.size()>100) {
		m_deme_diversity.pop_front();
	}
	while(m_shannon_div.size()>100) {
		m_shannon_div.pop_front();
	}
	while(m_num_orgs_perf_reaction.size() > 100) {
		m_num_orgs_perf_reaction.pop_front();
	}
  while (m_shannon_div_norm.size() > 100) {
		m_shannon_div_norm.pop_front();
  }
  while (m_percent_reproductives.size() > 100) {
		m_percent_reproductives.pop_front();
  }

	if(m_deme_diversity.empty()) {
		df.Write(0.0, "Mean number of different reactions by deme [demereact]");
	} else {
		df.Write(std::accumulate(m_deme_diversity.begin(), m_deme_diversity.end(), 0.0)/m_deme_diversity.size(), "Mean number of different reactions by deme [demereact]");
	}
	if(m_switching.empty()) {
		df.Write(0.0, "Mean number of deme switch penalties per org  [orgpen]");
	} else {
		df.Write(std::accumulate(m_switching.begin(), m_switching.end(), 0.0)/m_switching.size(), "Mean number of deme switch penalties per org  [orgpen]");
	}
	if(m_shannon_div.empty()) {
		df.Write(0.0, "Mean shannon mutual information per deme [shannon]");
	} else {
		df.Write(std::accumulate(m_shannon_div.begin(), m_shannon_div.end(), 0.0)/m_shannon_div.size(), "Mean shannon mutual entropy [shannon]");
	}
  if(m_shannon_div_norm.empty()) {
		df.Write(0.0, "Mean shannon normalized mutual information per deme [shannonnorm]");
	} else {
		df.Write(std::accumulate(m_shannon_div_norm.begin(), m_shannon_div_norm.end(), 0.0)/m_shannon_div_norm.size(), "Mean shannon normalalized mutual information [shannonnorm]");
	}
	if(m_num_orgs_perf_reaction.empty()) {
		df.Write(0.0, "Mean number of orgs that perform a reaction [meanreact]");
	} else {
		df.Write(std::accumulate(m_num_orgs_perf_reaction.begin(), m_num_orgs_perf_reaction.end(), 0.0)/m_num_orgs_perf_reaction.size(), "Mean number of orgs that perform a reaction [meanreact]");
	}
  if(m_percent_reproductives.empty()) {
		df.Write(0.0, "Mean percent of orgs that replicate [meanperrepros]");
	} else {
		df.Write(std::accumulate(m_percent_reproductives.begin(), m_percent_reproductives.end(), 0.0)/m_percent_reproductives.size(), "Mean percent of orgs that replicate [meanperrepros]");
	}


  df.Endl();
}

/*! Prints the genotype ids of all organisms within the maximally-fit deme.
 */
void cStats::PrintWinningDeme(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteComment("Genotype IDs of the constituent organisms within each deme.");
  df.WriteTimeStamp();
  df.WriteColumnDesc("Update [update]");
  df.WriteColumnDesc("Deme id [demeid]");
  df.WriteColumnDesc("Deme fitness [fitness]");
  df.WriteColumnDesc("Number of unique genomes in deme [uniq]");
  df.WriteColumnDesc("Genome ID [genomeids]");
  df.FlushComments();

  std::pair<int, double> max_element = std::make_pair(0, 0.0);
  bool found_max = false;
  for (int i=0; i<(int)m_deme_fitness.size(); ++i) {
    if (m_deme_fitness[i] > max_element.second) {
      max_element.first = i;
      max_element.second = m_deme_fitness[i];
      found_max = true;
    }
  }

  if(!found_max) {
    return;
  }

  df.WriteAnonymous(GetUpdate());
  df.WriteAnonymous(max_element.first);
  df.WriteAnonymous(max_element.second);

  cDeme& deme = m_world->GetPopulation().GetDeme(max_element.first);

  std::set<int> uniq;
  std::vector<int> genotypes;

  for(int i=0; i<deme.GetSize(); ++i) {
    cOrganism* org=deme.GetOrganism(i);
    if(org != 0) {
      genotypes.push_back(org->GetBioGroup("genotype")->GetID());
      uniq.insert(org->GetBioGroup("genotype")->GetID());
    }
  }

  df.WriteAnonymous((int)uniq.size());

  for(int i=0; i<(int)genotypes.size(); ++i) {
    df.WriteAnonymous(genotypes[i]);
  }
  df.Endl();
}


/*! Record information about an organism migrating from this population.
 */
void cStats::OutgoingMigrant(const cOrganism* org) {
	m_outgoing.Add(1);
}

/*! Record information about an organism migrating into this population.
 */
void cStats::IncomingMigrant(const cOrganism* org) {
	m_incoming.Add(1);
}

/*! Print multiprocess data.
 */
void cStats::PrintMultiProcessData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Avida-MP data");
  df.WriteTimeStamp();
  df.Write(GetUpdate(), "Update [update]");
	df.Write(m_outgoing.Count(), "Outgoing migrants [outgoing]");
	df.Write(m_incoming.Count(), "Incoming migrants [incoming]");
	df.Endl();

	m_outgoing.Clear();
	m_incoming.Clear();
}

/*! Track profiling data.
 */
void cStats::ProfilingData(const profiling_stats_t& pf) {
	for(profiling_stats_t::const_iterator i=pf.begin(); i!=pf.end(); ++i) {
		m_profiling[i->first].Add(i->second);
	}
}

/*! Print profiling data.
 */
void cStats::PrintProfilingData(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);

  df.WriteComment("Profiling statistics");
  df.WriteTimeStamp();
	df.Write(GetUpdate(), "Update [update]");

	for(avg_profiling_stats_t::iterator i=m_profiling.begin(); i!=m_profiling.end(); ++i) {
		df.Write(i->second.Average(), i->first.c_str());
	}
	df.Endl();

	m_profiling.clear();
}

/*! Print organism location.
*/
void cStats::PrintOrganismLocation(const cString& filename) {
	cDataFile& df = m_world->GetDataFile(filename);
	
  df.WriteComment("Organism location data");
  df.WriteTimeStamp();
	
	for(int i=0; i<m_world->GetPopulation().GetSize(); ++i) {
		cPopulationCell& cell = m_world->GetPopulation().GetCell(i);
		if(cell.IsOccupied()) {
			df.Write(GetUpdate(), "Update [update]");
			df.Write(i, "Cell ID [cellid]");
			df.Write(cell.GetOrganism()->GetID(), "Organism ID [orgid]");
			df.Endl();
		}
	}	
}

// Records information about mates that are chosen from the birth chamber
void cStats::RecordSuccessfulMate(cBirthEntry& successful_mate, cBirthEntry& chooser) {
  //Check if we need to resize the array of successful mates, and re-size it if needed
  int array_size = m_successful_mates.GetSize();
  if (array_size <= m_num_successful_mates) {
    m_successful_mates.Resize(m_num_successful_mates + 1);
    m_choosers.Resize(m_num_successful_mates + 1);
  }
  
  m_successful_mates[m_num_successful_mates] = successful_mate;
  m_choosers[m_num_successful_mates] = chooser;
  
  m_num_successful_mates++;
}

// Records information about mates that are chosen from the birth chamber
void cStats::PrintSuccessfulMates(cString& filename) {
  cDataFile& df = m_world->GetDataFile(filename);
  df.WriteTimeStamp();
  df.WriteComment("First half of each line gives information about the 'chosen' mate");
  df.WriteComment("Second half of each line gives information about the 'chooser'");
  df.WriteComment(cBirthEntry::GetPhenotypeStringFormat());
  df.Endl();  
  std::ofstream& df_stream = df.GetOFStream();
  for (int i = 0; i < m_num_successful_mates; i++) {
    df_stream << m_successful_mates[i].GetPhenotypeString() << " " << m_choosers[i].GetPhenotypeString() << endl;
  }
  m_world->GetDataFileManager().Remove(filename);
}

//Prints out average data only for the males in the population (MATING_TYPES option should be turned on)
void cStats::PrintMaleAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Male Average Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_male_fitness.Average(),        "Fitness");
  df.Write(sum_male_gestation.Average(),      "Gestation Time");
  df.Write(sum_male_merit.Average(),          "Merit");
  df.Write(sum_male_creature_age.Average(),   "Creature Age");
  df.Write(sum_male_generation.Average(),     "Generation");
  df.Write(sum_male_size.Average(),           "Genome Length");
  
  df.Endl();
}

//Prints out average data only for the females in the population (MATING_TYPES option should be turned on)
void cStats::PrintFemaleAverageData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Female Average Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_female_fitness.Average(),        "Fitness");
  df.Write(sum_female_gestation.Average(),      "Gestation Time");
  df.Write(sum_female_merit.Average(),          "Merit");
  df.Write(sum_female_creature_age.Average(),   "Creature Age");
  df.Write(sum_female_generation.Average(),     "Generation");
  df.Write(sum_female_size.Average(),           "Genome Length");
  
  df.Endl();
}

void cStats::PrintMaleErrorData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Male Standard Error Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_male_fitness.StdError(),        "Fitness");
  df.Write(sum_male_gestation.StdError(),      "Gestation Time");
  df.Write(sum_male_merit.StdError(),          "Merit");
  df.Write(sum_male_creature_age.StdError(),   "Creature Age");
  df.Write(sum_male_generation.StdError(),     "Generation");
  df.Write(sum_male_size.StdError(),           "Genome Length");
  
  df.Endl();
}

void cStats::PrintFemaleErrorData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Female Standard Error Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_female_fitness.StdError(),        "Fitness");
  df.Write(sum_female_gestation.StdError(),      "Gestation Time");
  df.Write(sum_female_merit.StdError(),          "Merit");
  df.Write(sum_female_creature_age.StdError(),   "Creature Age");
  df.Write(sum_female_generation.StdError(),     "Generation");
  df.Write(sum_female_size.StdError(),           "Genome Length");
  
  df.Endl();
}

void cStats::PrintMaleVarianceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Male Variance Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_male_fitness.Variance(),        "Fitness");
  df.Write(sum_male_gestation.Variance(),      "Gestation Time");
  df.Write(sum_male_merit.Variance(),          "Merit");
  df.Write(sum_male_creature_age.Variance(),   "Creature Age");
  df.Write(sum_male_generation.Variance(),     "Generation");
  df.Write(sum_male_size.Variance(),           "Genome Length");
  
  df.Endl();
}

void cStats::PrintFemaleVarianceData(const cString& filename)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Female Variance Data");
  df.WriteTimeStamp();
  
  df.Write(m_update,                          "Update");
  df.Write(sum_female_fitness.Variance(),        "Fitness");
  df.Write(sum_female_gestation.Variance(),      "Gestation Time");
  df.Write(sum_female_merit.Variance(),          "Merit");
  df.Write(sum_female_creature_age.Variance(),   "Creature Age");
  df.Write(sum_female_generation.Variance(),     "Generation");
  df.Write(sum_female_size.Variance(),           "Genome Length");
  
  df.Endl();
  
}

void cStats::PrintMaleInstructionData(const cString& filename, const cString& inst_set)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Male instruction execution data");
  df.WriteTimeStamp();
  
  df.Write(m_update, "Update");
  
  for (int i = 0; i < m_is_pred_exe_inst_map[inst_set].GetSize(); i++) {
    df.Write(m_is_male_exe_inst_map[inst_set][i].Sum(), m_is_inst_names_map[inst_set][i]);
  }
  
  df.Endl();
}

void cStats::PrintFemaleInstructionData(const cString& filename, const cString& inst_set)
{
  cDataFile& df = m_world->GetDataFile(filename);
  
  df.WriteComment("Female instruction execution data");
  df.WriteTimeStamp();
  
  df.Write(m_update, "Update");
  
  for (int i = 0; i < m_is_pred_exe_inst_map[inst_set].GetSize(); i++) {
    df.Write(m_is_female_exe_inst_map[inst_set][i].Sum(), m_is_inst_names_map[inst_set][i]);
  }
  
  df.Endl();  
}

void cStats::PrintMicroTraces(tSmartArray<char>& exec_trace, int birth_update, int org_id, int ft, int gen_id)
{
  int death_update = GetUpdate();
  cDataFile& df = m_world->GetDataFile("microtrace.dat");
  
  if (!df.HeaderDone()) {
    df.WriteComment("Trace Execution Data");
    df.WriteTimeStamp();
    df.WriteComment("DeathUpdate");
    df.WriteComment("BirthUpdate");
    df.WriteComment("OrgID");
    df.WriteComment("GenotypeID");
    df.WriteComment("ForageTarget");
    df.Endl();
  }
  
  std::ofstream& fp = df.GetOFStream();
  fp << death_update << "," << birth_update << "," << org_id << "," << gen_id << "," << ft << ",";
  for (int i = 0; i < exec_trace.GetSize(); i++) {
    fp << exec_trace[i];
  }
  fp << endl;
}

void cStats::UpdateTopNavTrace(cOrganism* org)
{
  // 'best' org is the one among the orgs with the highest reaction achieved that reproduced in the least number of cycles
  // using cycles, so any inst executions in parallel multi-threads are only counted as one exec
  int best_reac = -1;
  tArray<int> reaction_count = org->GetPhenotype().GetCurReactionCount();
  for (int i = reaction_count.GetSize() -1; i >= 0; i--) {
    if (reaction_count[i] > 0) {
      best_reac = i;
      break;
    }
  }
  int cycle = org->GetPhenotype().GetTimeUsed();
  bool new_winner = false;
  if (best_reac >= topreac) {
    if (best_reac == topreac && cycle < topcycle) new_winner = true;
    else if (best_reac > topreac) new_winner = true;      
  }
  if (new_winner) {
    topreac = best_reac;
    topcycle = cycle;
    topgenid = org->GetBioGroup("genotype")->GetID();
    topid = org->GetID();
    
    tSmartArray<char> trace = org->GetHardware().GetMicroTrace();
    tSmartArray<int> traceloc = org->GetHardware().GetNavTraceLoc();
    tSmartArray<int> tracefacing = org->GetHardware().GetNavTraceFacing();
    tSmartArray<int> traceupdate = org->GetHardware().GetNavTraceUpdate();

    toptrace.Resize(trace.GetSize());
    topnavtraceloc.Resize(traceloc.GetSize());
    topnavtraceloc.SetAll(-1);
    topnavtracefacing.Resize(tracefacing.GetSize());
    topnavtracefacing.SetAll(-1);
    topnavtraceupdate.Resize(traceupdate.GetSize());
    topnavtraceupdate.SetAll(-1);
    
    assert(toptrace.GetSize() == topnavtraceloc.GetSize()); 
    assert(topnavtraceloc.GetSize() == topnavtracefacing.GetSize());
    assert(topnavtracefacing.GetSize() == topnavtraceupdate.GetSize());
    for (int i = 0; i < toptrace.GetSize(); i++) {
      toptrace[i] = trace[i];
      topnavtraceloc[i] = traceloc[i];
      topnavtracefacing[i] = tracefacing[i];
      topnavtraceupdate[i] = traceupdate[i];
    }
    
    tArray<int> reaction_cycles = org->GetPhenotype().GetFirstReactionCycles();
    tArray<int> reaction_execs = org->GetPhenotype().GetFirstReactionExecs();
    
    topreactioncycles.Resize(reaction_cycles.GetSize());
    topreactioncycles.SetAll(-1);
    topreactionexecs.Resize(reaction_execs.GetSize());
    topreactionexecs.SetAll(-1);
    topreactions.Resize(reaction_count.GetSize());
    topreactions.SetAll(0);
    
    assert(topreactions.GetSize() == topreactioncycles.GetSize());
    assert(topreactioncycles.GetSize() == topreactionexecs.GetSize());
    for (int i = 0; i < topreactions.GetSize(); i++) {
      topreactions[i] = reaction_count[i];
      topreactioncycles[i] = reaction_cycles[i];
      topreactionexecs[i] = reaction_execs[i];
    }
  }
  if (m_world->GetPopulation().GetTopNavQ().GetSize() <= 1) PrintTopNavTrace();
}

void cStats::PrintTopNavTrace()
{  
  cDataFile& df = m_world->GetDataFile("navtrace.dat");

  df.WriteComment("Org That Reproduced the Fastest (fewest cycles) Among Orgs with the Highest Reaction ID");
  df.WriteTimeStamp();
  df.WriteComment("GenotypeID");
  df.WriteComment("OrgID");
  df.WriteComment("Cycle at First Reproduction (parallel multithread execs = 1 cycle)");
  df.WriteComment("Reaction Counts at First Reproduction");
  df.WriteComment("CPU Cycle at First Trigger of Each Reaction");
  df.WriteComment("Exec Count at First Trigger (== index into execution trace and nav traces)");
  df.WriteComment("");
  df.WriteComment("Updates for each entry in each following trace (to match with res data)");
  df.WriteComment("CellIDs to First Reproduction");
  df.WriteComment("OrgFacings to First Reproduction");
  df.WriteComment("Execution Trace to First Reproduction");
  df.Endl();

  std::ofstream& fp = df.GetOFStream();

  if (topreactions.GetSize()) {
    fp << topgenid << " " << topid << " " << topcycle << " ";
    // reaction related
    for (int i = 0; i < topreactions.GetSize() - 1; i++) {
      fp << topreactions[i] << ",";
    }
    fp << topreactions[topreactions.GetSize() - 1] << " ";
    
    for (int i = 0; i < topreactioncycles.GetSize() - 1; i++) {
      fp << topreactioncycles[i] << ",";
    }
    fp << topreactioncycles[topreactioncycles.GetSize() - 1] << " ";
    
    for (int i = 0; i < topreactionexecs.GetSize() - 1; i++) {
      fp << topreactionexecs[i] << ",";
    }
    fp << topreactionexecs[topreactionexecs.GetSize() - 1] << " ";
    
    // instruction exec sequence related (printed in reverse order to get firs exec as first printed)
    for (int i = 0; i < topnavtraceupdate.GetSize() - 1; i++) {
      fp << topnavtraceupdate[i] << ",";
    }
    fp << topnavtraceupdate[topnavtraceupdate.GetSize() - 1] << " ";
    
    for (int i = 0; i < topnavtraceloc.GetSize() - 1; i++) {
      fp << topnavtraceloc[i] << ",";
    }
    fp << topnavtraceloc[topnavtraceloc.GetSize() - 1] << " ";
    
    for (int i = 0; i < topnavtracefacing.GetSize() - 1; i++) {
      fp << topnavtracefacing[i] << ",";
    }
    fp << topnavtracefacing[topnavtracefacing.GetSize() - 1] << " ";
    
    for (int i = 0; i < toptrace.GetSize(); i++) {
      fp << toptrace[i];
    }
    fp << endl;
  }
}

void cStats::PrintReproData(cOrganism* org)
{
  int update = GetUpdate();
  cDataFile& df = m_world->GetDataFile("repro_data.dat");
  
  if (!df.HeaderDone()) {
    df.WriteComment("Org Data up to First Reproduction");
    df.WriteTimeStamp();
    df.WriteComment("ReproUpdate");
    df.WriteComment("GenotypeID");
    df.WriteComment("OrgID");
    df.WriteComment("Age (updates)");
    df.WriteComment("TimeUsed (cycles)");
    df.WriteComment("NumExecutions (cycles * threads)");
    df.WriteComment("ReactionCounts");
    df.Endl();
  }

  std::ofstream& fp = df.GetOFStream();
  fp << update << " " << org->GetBioGroup("genotype")->GetID()<< " " << org->GetID() << " " << org->GetPhenotype().GetAge() << " " << org->GetPhenotype().GetTimeUsed() 
      << " " << org->GetPhenotype().GetNumExecs() << " ";
  tArray<int> reaction_count = org->GetPhenotype().GetCurReactionCount();
  for (int i = 0; i < reaction_count.GetSize() - 1; i++) {
    fp << reaction_count[i] << ",";
  }
  fp << reaction_count[reaction_count.GetSize() - 1] << endl;
}
