/*
 *  cStats.h
 *  Avida
 *
 *  Called "stats.hh" prior to 12/5/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1993-2002 California Institute of Technology.
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

#ifndef cStats_h
#define cStats_h

#include "avida/Avida.h"

#include "AvidaTools.h"

#include "avida/core/InstructionSequence.h"
#include "avida/data/Provider.h"

#include "cDoubleSum.h"
#include "cGenomeUtil.h"
#include "cIntSum.h"
#include "cOrganism.h"
#include "cRunningAverage.h"
#include "cRunningStats.h"
#include "nGeometry.h"
#include "tArray.h"
#include "tArrayMap.h"
#include "tDataManager.h"
#include "tMatrix.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>

class cWorld;
class cOrganism;
class cOrgMessage;
class cOrgMessagePredicate;
class cOrgMovementPredicate;
class cDeme;
class cGermline;

using namespace Avida;


struct flow_rate_tuple {
  cIntSum orgCount;
  cIntSum eventsKilled;
  cIntSum attemptsToKillEvents;
  cDoubleSum AvgEnergyUsageRatio;
  cIntSum totalBirths;
  cIntSum currentSleeping;
};

struct s_inst_circumstances {
  int update;
  int gr_id;
  int gr_size;
  double res_level;
  double odds_immigrants;
  double odds_offspring_own;
  double odds_offspring_others;
  int tol_immigrants;
  int tol_own;
  int tol_others;
  int tol_max;
}; // @JJB

class cStats : public Data::ArgumentedProvider
{
private:
  cWorld* m_world;

  tDataManager<cStats> m_data_manager;
  
  
  // --------  Data Provider Support  ---------
  struct ProvidedData
  {
    Apto::String description;
    Apto::Functor<Data::PackagePtr, Apto::NullType> GetData;
    
    ProvidedData() { ; }
    ProvidedData(const Apto::String& desc, Apto::Functor<Data::PackagePtr, Apto::NullType> func)
      : description(desc), GetData(func) { ; } 
  };
  Apto::Map<Apto::String, ProvidedData> m_provided_data;
  mutable Data::ConstDataSetPtr m_provides;


  // --------  Time scales  ---------
  int m_update;
  double avida_time;



  // --------  Organism Sums  ---------  (Cleared and resummed by population each update)
  cDoubleSum sum_merit;
  cDoubleSum sum_mem_size;
  cDoubleSum sum_creature_age;
  cDoubleSum sum_generation;

  cDoubleSum sum_neutral_metric;
  cDoubleSum sum_lineage_label;

  cRunningStats sum_copy_mut_rate;
  cRunningStats sum_log_copy_mut_rate;

  cRunningStats sum_div_mut_rate;
  cRunningStats sum_log_div_mut_rate;


  // --------  Genotype Sums  ---------  (Cleared and resummed by population each update)
  cDoubleSum sum_gestation;
  cDoubleSum sum_fitness;
  cDoubleSum sum_repro_rate;

  // calculates a running average over the actual replication rate
  // given by the number of births per update
  cRunningAverage rave_true_replication_rate;

  cDoubleSum sum_size;
  cDoubleSum sum_copy_size;
  cDoubleSum sum_exe_size;



  // --------  Instruction Counts  ---------
  tArrayMap<cString, tArray<cString> > m_is_inst_names_map;
  tArrayMap<cString, tArray<cIntSum> > m_is_exe_inst_map;
  tArray<tSmartArray<s_inst_circumstances> > m_is_tolerance_exe_insts; // @JJB

  // --------  Calculated Stats  ---------
  double entropy;
  double species_entropy;
  double dom_fidelity;
  double ave_fidelity;

  // For tracking of advantageous mutations
  double max_viable_fitness;

  // --------  Dominant Genotype  ---------
  double max_fitness;
  double max_merit;
  int max_gestation_time;
  int max_genome_length;
  double min_fitness;
  double min_merit;
  int min_gestation_time;
  int min_genome_length;


  // --------  Population Stats  ---------
  int num_births;
  int num_deaths;
  int num_breed_in;
  int num_breed_true;
  int num_breed_true_creatures;
  int num_creatures;
  int num_executed;
  int num_parasites;
  int num_no_birth_creatures;
  int num_single_thread_creatures;
  int num_multi_thread_creatures;
  int m_num_threads;
  int num_modified;

  int tot_organisms;
  int tot_executed;

  // --------  Parasite Task Stats  ---------
  tArray<int> tasks_host_current;
  tArray<int> tasks_host_last;
  tArray<int> tasks_parasite_current;
  tArray<int> tasks_parasite_last;


  // --------  Organism Task Stats  ---------
  tArray<int> task_cur_count;
  tArray<int> task_last_count;
  tArray<double> task_cur_quality;
  tArray<double> task_last_quality;
  tArray<double> task_cur_max_quality;
  tArray<double> task_last_max_quality;
  tArray<int> task_exe_count;
  tArray<int> new_task_count;
  tArray<int> prev_task_count;
  tArray<int> cur_task_count;
  tArray<int> new_reaction_count;

  tArray<int> task_internal_cur_count;
  tArray<int> task_internal_last_count;
  tArray<double> task_internal_cur_quality;
  tArray<double> task_internal_last_quality;
  tArray<double> task_internal_cur_max_quality;
  tArray<double> task_internal_last_max_quality;

  tArray<int> m_reaction_cur_count;
  tArray<int> m_reaction_last_count;
  tArray<double> m_reaction_cur_add_reward;
  tArray<double> m_reaction_last_add_reward;
  tArray<int> m_reaction_exe_count;

  tArray<double> resource_count;
  tArray<int> resource_geometry;
  tArray< tArray<double> > spatial_res_count;

  tArray<cString> task_names;
  tArray<cString> reaction_names;
  tArray<cString> resource_names;

  // --------  Resampling Stats  ---------
  int num_resamplings;
  int num_failedResamplings;


  // --------  State Variables  ---------
  int last_update;


  // --------  Sense Instruction Stats  ---------
  int sense_size;
  tArray<int> sense_last_count;
  tArray<int> sense_last_exe_count;
  tArray<cString> sense_names;


  // --------  Competition Stats  ---------
  tArray<double> avg_trial_fitnesses;
  double avg_competition_fitness;
  double min_competition_fitness;
  double max_competition_fitness;
  double avg_competition_copied_fitness;
  double min_competition_copied_fitness;
  double max_competition_copied_fitness;
  int num_orgs_replicated;


  // --------  Deme Stats  ---------
  cIntSum sum_deme_age;
  cIntSum sum_deme_birth_count;
  cIntSum sum_deme_last_birth_count;
  cIntSum sum_deme_org_count;
  cIntSum sum_deme_last_org_count;
  cIntSum sum_deme_generation;
  cIntSum sum_deme_gestation_time;
  cDoubleSum sum_deme_normalized_time_used;
  cDoubleSum sum_deme_merit;
  cDoubleSum sum_deme_generations_per_lifetime;
  int m_num_occupied_demes;

  cIntSum sum_deme_events_killed;
  cIntSum sum_deme_events_kill_attempts;

  cDoubleSum EnergyTestamentToFutureDeme;
  cDoubleSum EnergyTestamentToNeighborOrganisms;
  cDoubleSum EnergyTestamentToDemeOrganisms;
  cDoubleSum EnergyTestamentAcceptedByOrganisms;
  cDoubleSum EnergyTestamentAcceptedByDeme;

  //(event flow rate, (deme pop size, events killed))
  std::map<int, flow_rate_tuple > flow_rate_tuples;


  // --------  Deme Predicate Stats  ---------
  tMatrix<int> relative_pos_event_count;
  tMatrix<int> relative_pos_pred_sat;
	std::map<cString, int> demeResourceThresholdPredicateMap;


  // --------  Speculative Execution Stats  ---------
  int m_spec_total;
  int m_spec_num;
  int m_spec_waste;


  // --------  Organism Kill Stats  ---------
  cIntSum sum_orgs_killed;
  cIntSum sum_unoccupied_cell_kill_attempts;
  cIntSum sum_cells_scanned_at_kill;


  // --------  Migration Stats  ---------
  int num_migrations;


public:
  cStats(cWorld* world);
  ~cStats() { ; }

  
  // Data::Provider
  Data::ConstDataSetPtr Provides() const;
  void UpdateProvidedValues(Update current_update);
  Apto::String DescribeProvidedValue(const Apto::String& data_id) const;

  // Data::ArgumentedProvider
  void SetActiveArguments(const Data::DataID& data_id, Data::ConstArgumentSetPtr args);
  Data::ConstArgumentSetPtr GetValidArguments(const Data::DataID& data_id) const;
  bool IsValidArgument(const Data::DataID& data_id, Data::Argument arg) const;
  
  Data::PackagePtr GetProvidedValueForArgument(const Data::DataID& data_id, const Data::Argument& arg) const;
  
  // cStats
  void ProcessUpdate();

  inline void SetCurrentUpdate(int new_update) { m_update = new_update; }
  inline void IncCurrentUpdate() { m_update++; }

  // Accessors...
  int GetUpdate() const { return m_update; }
  double GetGeneration() const { return SumGeneration().Average(); }

  int GetSenseSize() const { return sense_size; }

  // Settings...

  inline void SetNumGenotypes(int new_genotypes, int num_historic);
  inline void SetNumCreatures(int new_creatures) { num_creatures = new_creatures; }
  inline void SetBreedTrueCreatures(int in_num_breed_true_creatures);
  inline void SetNumParasites(int in_num_parasites);
  inline void SetNumNoBirthCreatures(int in_num_no_birth_creatures);
  inline void SetNumSingleThreadCreatures(int in_num_single_thread_creatures);
  inline void SetNumMultiThreadCreatures(int in_num_multi_thread_creatures);
  inline void SetNumThreads(int in_num_threads) { m_num_threads = in_num_threads; }
  inline void SetNumModified(int in_num_modified);

  void SetMaxFitness(double in_max_fitness) { max_fitness = in_max_fitness; }
  void SetMaxMerit(double in_max_merit) { max_merit = in_max_merit; }
  void SetMaxGestationTime(int in_max_gestation_time) { max_gestation_time = in_max_gestation_time; }
  void SetMaxGenomeLength(int in_max_genome_length) { max_genome_length = in_max_genome_length; }

  void SetMinFitness(double in_min_fitness) { min_fitness = in_min_fitness; }
  void SetMinMerit(double in_min_merit) { min_merit = in_min_merit; }
  void SetMinGestationTime(int in_min_gestation_time) { min_gestation_time = in_min_gestation_time; }
  void SetMinGenomeLength(int in_min_genome_length) { min_genome_length = in_min_genome_length; }

  void SetEntropy(double in_entropy) { entropy = in_entropy; }
  void SetSpeciesEntropy(double in_ent) { species_entropy = in_ent; }

  cDoubleSum& SumFitness()       { return sum_fitness; }
  cDoubleSum& SumGestation()     { return sum_gestation; }
  cDoubleSum& SumMerit()         { return sum_merit; }
  cDoubleSum& SumReproRate()     { return sum_repro_rate; }

  cDoubleSum& SumCreatureAge()   { return sum_creature_age; }
  cDoubleSum& SumGeneration()    { return sum_generation; }

  cDoubleSum& SumNeutralMetric() { return sum_neutral_metric; }
  cDoubleSum& SumLineageLabel()  { return sum_lineage_label; }
  cRunningStats& SumCopyMutRate()   { return sum_copy_mut_rate; }
  cRunningStats& SumLogCopyMutRate()   { return sum_log_copy_mut_rate; }
  cRunningStats& SumDivMutRate()   { return sum_div_mut_rate; }
  cRunningStats& SumLogDivMutRate()   { return sum_log_div_mut_rate; }

  cDoubleSum& SumSize()          { return sum_size; }
  cDoubleSum& SumCopySize()      { return sum_copy_size; }
  cDoubleSum& SumExeSize()       { return sum_exe_size; }
  cDoubleSum& SumMemSize()       { return sum_mem_size; }

  //deme
  cIntSum& SumDemeAge()          { return sum_deme_age; }
  cIntSum& SumDemeBirthCount()   { return sum_deme_birth_count; }
  cIntSum& SumDemeLastBirthCount()   { return sum_deme_last_birth_count; }
  cIntSum& SumDemeOrgCount()     { return sum_deme_org_count; }
  cIntSum& SumDemeLastOrgCount()     { return sum_deme_last_org_count; }
  cIntSum& SumDemeGeneration()   { return sum_deme_generation; }
  cIntSum& SumDemeGestationTime()   { return sum_deme_gestation_time; }
  cDoubleSum& SumDemeNormalizedTimeUsed()   { return sum_deme_normalized_time_used; }
  cDoubleSum& SumDemeMerit()   { return sum_deme_merit; }
  cDoubleSum& SumDemeGenerationsPerLifetime()   { return sum_deme_generations_per_lifetime; }

  cIntSum& SumDemeEventsKilled()          { return sum_deme_events_killed; }
  cIntSum& SumDemeAttemptsToKillEvents()          { return sum_deme_events_kill_attempts; }

  cDoubleSum& SumEnergyTestamentToFutureDeme() { return EnergyTestamentToFutureDeme;}
  cDoubleSum& SumEnergyTestamentToNeighborOrganisms() { return EnergyTestamentToNeighborOrganisms; }
  cDoubleSum& SumEnergyTestamentToDemeOrganisms() { return EnergyTestamentToDemeOrganisms; }
  cDoubleSum& SumEnergyTestamentAcceptedByOrganisms() { return EnergyTestamentAcceptedByOrganisms; }
  cDoubleSum& SumEnergyTestamentAcceptedByDeme() { return EnergyTestamentAcceptedByDeme; }

  std::map<int, flow_rate_tuple >&  FlowRateTuples() { return flow_rate_tuples; }

  void ZeroInst();
  tArray<cIntSum>& InstExeCountsForInstSet(const cString& inst_set) { return m_is_exe_inst_map[inst_set]; }

  // And constant versions of the above...
  const cDoubleSum& SumFitness() const       { return sum_fitness; }
  const cDoubleSum& SumGestation() const     { return sum_gestation; }
  const cDoubleSum& SumMerit() const         { return sum_merit; }
  const cDoubleSum& SumReproRate() const     { return sum_repro_rate; }

  const cDoubleSum& SumCreatureAge() const   { return sum_creature_age; }
  const cDoubleSum& SumGeneration() const    { return sum_generation; }

  const cDoubleSum& SumNeutralMetric() const { return sum_neutral_metric; }
  const cDoubleSum& SumLineageLabel() const  { return sum_lineage_label; }
  const cRunningStats& SumCopyMutRate() const   { return sum_copy_mut_rate; }
  const cRunningStats& SumLogCopyMutRate() const{ return sum_log_copy_mut_rate; }
  const cRunningStats& SumDivMutRate() const   { return sum_div_mut_rate; }
  const cRunningStats& SumLogDivMutRate() const{ return sum_log_div_mut_rate; }

  const cDoubleSum& SumSize() const          { return sum_size; }
  const cDoubleSum& SumCopySize() const      { return sum_copy_size; }
  const cDoubleSum& SumExeSize() const       { return sum_exe_size; }
  const cDoubleSum& SumMemSize() const       { return sum_mem_size; }

  //deme
  const cIntSum& SumDemeAge() const          { return sum_deme_age; }
  const cIntSum& SumDemeBirthCount() const   { return sum_deme_birth_count; }
  const cIntSum& SumDemeLastBirthCount() const   { return sum_deme_last_birth_count; }
  const cIntSum& SumDemeOrgCount() const     { return sum_deme_org_count; }
  const cIntSum& SumDemeLastOrgCount() const     { return sum_deme_last_org_count; }
  const cIntSum& SumDemeGeneration() const   { return sum_deme_generation; }
  const cIntSum& SumDemeGestationTime() const  { return sum_deme_generation; }
  const cDoubleSum& SumDemeNormalizedTimeUsed() const  { return sum_deme_normalized_time_used; }
  const cDoubleSum& SumDemeMerit()  const  { return sum_deme_merit; }
  const cDoubleSum& SumDemeGenerationsPerLifetime() const  { return sum_deme_generations_per_lifetime; }

  const cIntSum& SumDemeEventsKilled() const          { return sum_deme_events_killed; }
  const cIntSum& SumDemeAttemptsToKillEvents() const  { return sum_deme_events_kill_attempts; }

  const cDoubleSum& SumEnergyTestamentToFutureDeme() const { return EnergyTestamentToFutureDeme;}
  const cDoubleSum& SumEnergyTestamentToNeighborOrganisms() const { return EnergyTestamentToNeighborOrganisms; }
  const cDoubleSum& SumEnergyTestamentToDemeOrganisms() const { return EnergyTestamentToDemeOrganisms; }
  const cDoubleSum& SumEnergyTestamentAcceptedByOrganisms() const { return EnergyTestamentAcceptedByOrganisms; }
  const cDoubleSum& SumEnergyTestamentAcceptedByDeme() const { return EnergyTestamentAcceptedByDeme; }

  const std::map<int, flow_rate_tuple >&  FlowRateTuples() const { return flow_rate_tuples; }

  void IncResamplings() { ++num_resamplings; }
  void IncFailedResamplings() { ++num_failedResamplings; }

  void CalcFidelity();

  void RecordBirth(bool breed_true);
  void RecordDeath() { num_deaths++; }

  void IncExecuted() { num_executed++; }

  void AddNumOrgsKilled(long num) { sum_orgs_killed.Add(num); }
	void AddNumUnoccupiedCellAttemptedToKill(long num) { sum_unoccupied_cell_kill_attempts.Add(num); }
  void AddNumCellsScannedAtKill(long num) { sum_cells_scanned_at_kill.Add(num); }
  void IncNumMigrations() { num_migrations++; }

  void AddCurTask(int task_num) { task_cur_count[task_num]++; }
  void AddCurHostTask(int task_num) { tasks_host_current[task_num]++; }
  void AddCurParasiteTask(int task_num) { tasks_parasite_current[task_num]++; }

  void AddCurTaskQuality(int task_num, double quality)
  {
	  task_cur_quality[task_num] += quality;
	  if (quality > task_cur_max_quality[task_num]) task_cur_max_quality[task_num] = quality;
  }
  void AddLastTask(int task_num) { task_last_count[task_num]++; }
  void AddLastHostTask(int task_num) { tasks_host_last[task_num]++; }
  void AddLastParasiteTask(int task_num) { tasks_parasite_last[task_num]++; }

  void AddLastTaskQuality(int task_num, double quality)
  {
	  task_last_quality[task_num] += quality;
	  if (quality > task_last_max_quality[task_num]) task_last_max_quality[task_num] = quality;
  }
  void AddNewTaskCount(int task_num) {new_task_count[task_num]++; }
  void AddOtherTaskCounts(int task_num, int prev_tasks, int cur_tasks) {
	  prev_task_count[task_num] += prev_tasks;
	  cur_task_count[task_num] += cur_tasks;
  }
  void AddNewReactionCount(int reaction_num) {new_reaction_count[reaction_num]++; }
  void IncTaskExeCount(int task_num, int task_count) { task_exe_count[task_num] += task_count; }
  void ZeroTasks();

  void AddLastSense(int) { /*sense_last_count[res_comb_index]++;*/ }
  void IncLastSenseExeCount(int, int) { /*sense_last_exe_count[res_comb_index]+= count;*/ }

  // internal resource bins and use of internal resources
  void AddCurInternalTask(int task_num) { task_internal_cur_count[task_num]++; }
  void AddCurInternalTaskQuality(int task_num, double quality)
  {
  	task_internal_cur_quality[task_num] += quality;
  	if(quality > task_internal_cur_max_quality[task_num])	task_internal_cur_max_quality[task_num] = quality;
  }
  void AddLastInternalTask(int task_num) { task_internal_last_count[task_num]++; }
  void AddLastInternalTaskQuality(int task_num, double quality)
  {
  	task_internal_last_quality[task_num] += quality;
  	if(quality > task_internal_last_max_quality[task_num]) task_internal_last_max_quality[task_num] = quality;
  }

  void AddCurReaction(int reaction) { m_reaction_cur_count[reaction]++; }
  void AddLastReaction(int reaction) { m_reaction_last_count[reaction]++; }
  void AddCurReactionAddReward(int reaction, double reward) { m_reaction_cur_add_reward[reaction] += reward; }
  void AddLastReactionAddReward(int reaction, double reward) { m_reaction_last_add_reward[reaction] += reward; }
  void IncReactionExeCount(int reaction, int count) { m_reaction_exe_count[reaction] += count; }
  void ZeroReactions();

  void SetResources(const tArray<double> &_in) { resource_count = _in; }
  void SetResourcesGeometry(const tArray<int> &_in) { resource_geometry = _in;}
  void SetSpatialRes(const tArray< tArray<double> > &_in) { spatial_res_count = _in; }

  void SetInstNames(const cString& inst_set, const tArray<cString>& names) { m_is_inst_names_map[inst_set] = names; }
  void SetReactionName(int id, const cString & name) { reaction_names[id] = name; }
  void SetResourceName(int id, const cString & name) { resource_names[id] = name; }

  void SetCompetitionTrialFitnesses(tArray<double> _in) { avg_trial_fitnesses = _in; }
  void SetCompetitionFitnesses(double _in_avg, double _in_min, double _in_max, double _in_cp_avg, double _in_cp_min, double _in_cp_max)
    { avg_competition_fitness = _in_avg; min_competition_fitness = _in_min; max_competition_fitness = _in_max;
      avg_competition_copied_fitness = _in_cp_avg; min_competition_copied_fitness = _in_cp_min; max_competition_copied_fitness = _in_cp_max; }
  void SetCompetitionOrgsReplicated(int _in) { num_orgs_replicated = _in; }

  void AddSpeculative(int spec) { m_spec_total += spec; m_spec_num++; }
  void AddSpeculativeWaste(int waste) { m_spec_waste += waste; }

  // Information retrieval section...

  int GetNumBirths() const          { return num_births; }
  int GetNumDeaths() const          { return num_deaths; }
  int GetBreedIn() const            { return num_breed_in; }
  int GetBreedTrue() const          { return num_breed_true; }
  int GetBreedTrueCreatures() const { return num_breed_true_creatures; }
  int GetNumCreatures() const       { return num_creatures; }
  int GetNumParasites() const       { return num_parasites; }
  int GetNumNoBirthCreatures() const{ return num_no_birth_creatures; }
  int GetNumSingleThreadCreatures() const { return num_single_thread_creatures; }
  int GetNumMultiThreadCreatures() const { return num_multi_thread_creatures; }
  int GetNumThreads() const { return m_num_threads; }
  int GetNumModified() const { return num_modified;}

  int GetTotCreatures() const       { return tot_organisms; }

  int GetTaskCurCount(int task_num) const { return task_cur_count[task_num]; }
  int GetTaskHostCurCount(int task_num) const { return tasks_host_current[task_num]; }
  int GetTaskParasiteCurCount(int task_num) const { return tasks_parasite_current[task_num]; }
  double GetTaskCurQuality(int task_num) const { return task_cur_quality[task_num]/(double)task_cur_count[task_num]; }

  int GetTaskLastCount(int task_num) const {return task_last_count[task_num];}
  int GetTaskLastHostCount(int task_num) const {return tasks_host_last[task_num];}
  int GetTaskLastParasiteCount(int task_num) const {return tasks_parasite_last[task_num];}
  double GetTaskLastQuality(int task_num) const {return task_last_quality[task_num]/(double)task_last_count[task_num];}

  double GetTaskMaxCurQuality(int task_num) const { return task_cur_max_quality[task_num];}
  double GetTaskMaxLastQuality(int task_num) const { return task_last_max_quality[task_num];}
  int GetTaskExeCount(int task_num) const { return task_exe_count[task_num]; }

  // internal resource bins and use of internal resources
  int GetInternalTaskCurCount(int task_num) const { return task_internal_cur_count[task_num]; }
  double GetInternalTaskCurQuality(int task_num) const { return task_internal_cur_quality[task_num]/(double)task_internal_cur_count[task_num]; }
  double GetInternalTaskMaxCurQuality(int task_num) const { return task_internal_cur_max_quality[task_num]; }
  int GetInternalTaskLastCount(int task_num) const { return task_internal_last_count[task_num]; }
  double GetInternalTaskLastQuality(int task_num) const { return task_internal_last_quality[task_num]/(double)task_internal_last_count[task_num]; }
  double GetInternalTaskMaxLastQuality(int task_num) const { return task_internal_last_max_quality[task_num]; }

  const tArray<int>& GetReactions() const { return m_reaction_last_count; }
  const tArray<double> & GetResources() const { return resource_count; }

  double GetAveReproRate() const  { return sum_repro_rate.Average(); }

  double GetAveMerit() const      { return sum_merit.Average(); }
  double GetAveCreatureAge() const{ return sum_creature_age.Average(); }
  double GetAveMemSize() const    { return sum_mem_size.Average(); }

  double GetAveNeutralMetric() const { return sum_neutral_metric.Average(); }
  double GetAveLineageLabel() const  { return sum_lineage_label.Average(); }
  double GetAveCopyMutRate() const   { return sum_copy_mut_rate.Mean(); }
  double GetAveLogCopyMutRate() const{ return sum_log_copy_mut_rate.Mean();}
  double GetAveDivMutRate() const   { return sum_div_mut_rate.Mean(); }
  double GetAveLogDivMutRate() const{ return sum_log_div_mut_rate.Mean();}

  double GetAveGestation() const { return sum_gestation.Average(); }
  double GetAveFitness() const   { return sum_fitness.Average(); }

  double GetAveSize() const       { return sum_size.Average(); }
  double GetAveCopySize() const   { return sum_copy_size.Average(); }
  double GetAveExeSize() const    { return sum_exe_size.Average(); }

  double GetEntropy() const        { return entropy; }
  double GetSpeciesEntropy() const { return species_entropy; }

  double GetMaxFitness() const { return max_fitness; }
  double GetMaxMerit() const { return max_merit; }
  int GetMaxGestationTime() const { return max_gestation_time; }
  int GetMaxGenomeLength() const { return max_genome_length; }

  double GetMinFitness() const { return min_fitness; }
  double GetMinMerit() const { return min_merit; }
  int GetMinGestationTime() const { return min_gestation_time; }
  int GetMinGenomeLength() const { return min_genome_length; }


  int GetResamplings() const { return num_resamplings;}  //AWC 06/29/06
  int GetFailedResamplings() const { return num_failedResamplings;}  //AWC 06/29/06

  int GetNumSenseSlots();

  double GetAveSpeculative() const { return (m_spec_num) ? ((double)m_spec_total / (double)m_spec_num) : 0.0; }
  int GetSpeculativeWaste() const { return m_spec_waste; }

  double GetAvgNumOrgsKilled() const { return sum_orgs_killed.Average(); }
  double GetAvgNumCellsScannedAtKill() const { return sum_cells_scanned_at_kill.Average(); }
  int GetNumMigrations() const { return num_migrations; }

  // this value gets recorded when a creature with the particular
  // fitness value gets born. It will never change to a smaller value,
  // i.e., when the maximum fitness in the population drops, this value will
  // still stay up.
  double GetMaxViableFitness() const { return max_viable_fitness; }

  // User-defined datafile...
  void PrintDataFile(const cString& filename, const cString& format, char sep=' ');

  // Public calls to output data files (for events)
  void PrintAverageData(const cString& filename);
  void PrintDemeAverageData(const cString& filename);
  void PrintFlowRateTuples(const cString& filename);
  void PrintErrorData(const cString& filename);
  void PrintVarianceData(const cString& filename);
  void PrintParasiteData(const cString& filename);
  void PrintStatsData(const cString& filename);
  void PrintCountData(const cString& filename);
  void PrintThreadsData(const cString& filename);
	void PrintMessageData(const cString& filename);
  void PrintInterruptData(const cString& filename);
  void PrintTotalsData(const cString& filename);
  void PrintTasksData(const cString& filename);
  void PrintHostTasksData(const cString& filename);
  void PrintParasiteTasksData(const cString& filename);
  void PrintTasksExeData(const cString& filename);
  void PrintTasksQualData(const cString& filename);
  void PrintDynamicMaxMinData(const cString& filename);
  void PrintNewTasksData(const cString& filename);
  void PrintNewReactionData(const cString& filename);
  void PrintNewTasksDataPlus(const cString& filename);
  void PrintReactionData(const cString& filename);
  void PrintReactionExeData(const cString& filename);
  void PrintCurrentReactionData(const cString& filename);
  void PrintReactionRewardData(const cString& filename);
  void PrintCurrentReactionRewardData(const cString& filename);
  void PrintResourceData(const cString& filename);
  void PrintSpatialResData(const cString& filename, int i);
  void PrintTimeData(const cString& filename);
  void PrintDivideMutData(const cString& filename);
  void PrintMutationRateData(const cString& filename);
  void PrintInstructionData(const cString& filename, const cString& inst_set);
  void PrintSenseData(const cString& filename);
  void PrintSenseExeData(const cString& filename);
  void PrintInternalTasksData(const cString& filename);
  void PrintInternalTasksQualData(const cString& filename);
  void PrintSleepData(const cString& filename);
  void PrintCompetitionData(const cString& filename);
  void PrintCellVisitsData(const cString& filename);
  void PrintExtendedTimeData(const cString& filename);
  void PrintNumOrgsKilledData(const cString& filename);
  void PrintMigrationData(const cString& filename);
  void PrintGroupsFormedData(const cString& filename);
  void PrintGroupIds(const cString& filename);
  void PrintTargets(const cString& filename);
  void PrintGroupTolerance(const cString& filename); // @JJB
  void PrintToleranceInstructionData(const cString& filename); // @JJB
  void PrintToleranceData(const cString& filename); // @JJB

  // deme predicate stats
  void IncEventCount(int x, int y);
  void IncPredSat(int cell_id);
  void PrintPredSatFracDump(const cString& filename);

	void AddDemeResourceThresholdPredicate(cString& name);
	void IncDemeResourceThresholdPredicate(cString& name);
	void PrintDemeResourceThresholdPredicate(const cString& filename);

  void addOrgLocations(std::vector<std::pair<int, int> >);
  void PrintDemeRepOrgLocation(const cString& filename);

  // -------- Messaging support --------
public:
  //! Type for a list of pointers to message predicates.
  typedef std::vector<cOrgMessagePredicate*> message_pred_ptr_list;

  //! Called for every message successfully sent anywhere in the population.
  void SentMessage(const cOrgMessage& msg);
  //! Adds a predicate that will be evaluated for each message.
  void AddMessagePredicate(cOrgMessagePredicate* predicate);
  //! Removes a predicate.
  void RemoveMessagePredicate(cOrgMessagePredicate* predicate);
  //! Prints information regarding messages that "passed" their predicate.
  void PrintPredicatedMessages(const cString& filename);
	//! Log a message.
	void LogMessage(const cOrgMessage& msg, bool dropped, bool lost);
	//! Prints logged messages.
	void PrintMessageLog(const cString& filename);

protected:
  /*! List of all active message predicates.  The idea here is that the predicates,
  rather than cStats / cOrgMessage / etc., do the tracking of particular messages
  of interest. */
  message_pred_ptr_list m_message_predicates;
	//! Type to store logged messages.
	struct message_log_entry_t {
		message_log_entry_t(int u, int de, int s, int d, unsigned int md, unsigned int ml, bool dr, bool l)
		:	update(u), deme(de), src_cell(s), dst_cell(d), msg_data(md), msg_label(ml), dropped(dr), lost(l) {
		}
		int update, deme, src_cell, dst_cell;
		unsigned int msg_data, msg_label;
		bool dropped, lost;
	};
	typedef std::vector<message_log_entry_t> message_log_t; //!< Type for message log.
	message_log_t m_message_log; //!< Log for messages.

  // -------- End messaging support --------


  // -------- Movement support -------------
public:
  //! Type for a list of pointers to movement predicates.
  typedef std::vector<cOrgMovementPredicate*> movement_pred_ptr_list;
  void Move(cOrganism& org);
  void AddMovementPredicate(cOrgMovementPredicate* predicate);
protected:
  movement_pred_ptr_list m_movement_predicates;
  // -------- End movement support --------

  // -------- Tolerance support --------
public:
  void PushToleranceInstExe(int tol_inst, int group_id, int group_size, double resource_level, double odds_immi,
              double odds_own, double odds_others, int tol_immi, int tol_own, int tol_others, int tol_max); // @JJB
  void ZeroToleranceInst(); // @JJB


  // -------- Deme replication support --------
public:
  //! Called immediately prior to deme replacement.
  void DemePreReplication(cDeme& source_deme, cDeme& target_deme);
  //! Called immediately after deme replacement.
  void DemePostReplication(cDeme& source_deme, cDeme& target_deme);
  //! Called immediately prior to germline replacement.
  void GermlineReplication(cGermline& source_germline, cGermline& target_germline);
  //! Print statistics about deme replication.
  void PrintDemeReplicationData(const cString& filename);
  


	void PrintDemeTreatableReplicationData(const cString& filename);
	void PrintDemeUntreatableReplicationData(const cString& filename);
	void PrintDemeTreatableCount(const cString& filename);

  //! Print statistics about germlines.
  void PrintGermlineData(const cString& filename);
  //! Accessor for average "generation" of germlines.
  double GetAveGermlineGeneration() const { return m_germline_generation.Average(); }
  /*! Typedef of a data structure to track deme founders.
    * Map of deme id -> {founder genotype id_0, id_1,... id_{deme propagule size}} */
  typedef std::map<int, std::vector<int> > t_founder_map;
  //! Print the genotype IDs for the founders of demes that have recently been "born."
  void PrintDemeFoundersData(const cString& filename);

  void PrintPerDemeTasksData(const cString& filename);
  void PrintPerDemeTasksExeData(const cString& filename);
  void PrintAvgDemeTasksExeData(const cString& filename);
  void PrintAvgTreatableDemeTasksExeData(const cString& filename);
  void PrintAvgUntreatableDemeTasksExeData(const cString& filename);
  void PrintPerDemeReactionData(const cString& filename);
  void PrintDemeTasksData(const cString& filename);
  void PrintDemeTasksExeData(const cString& filename);
  void PrintDemeReactionData(const cString& filename);
  void PrintDemeOrgTasksData(const cString& filename);
  void PrintDemeOrgTasksExeData(const cString& filename);
  void PrintDemeOrgReactionData(const cString& filename);
  void PrintDemeCurrentTaskExeData(const cString& filename);
  void PrintCurrentTaskCounts(const cString& filename);
  void PrintPerDemeGenPerFounderData(const cString& filename);
	void PrintDemeMigrationSuicidePoints(const cString& filename);
	void PrintDemeReactionDiversityReplicationData(const cString& filename);
  void PrintWinningDeme(const cString& filename);

  void IncNumOccupiedDemes() { m_num_occupied_demes++; }
  void ClearNumOccupiedDemes() { m_num_occupied_demes = 0; }
  int GetNumOccupiedDemes() { return m_num_occupied_demes; }



protected:
  int m_deme_num_repls; //!< Number of deme replications since last PrintDemeReplicationData.
	int m_total_deme_num_repls; //!< Total number of deme replications ever.
  cDoubleSum m_deme_gestation_time; //!< Gestation time for demes - mean age at deme replication.
  cDoubleSum m_deme_births; //!< Mean number of births in replicated demes.
  cDoubleSum m_deme_merit; //!< Mean merit of replicated demes.
  cDoubleSum m_deme_generation; //!< Mean generation of replicated demes.
	cDoubleSum m_deme_density; //!< Mean density of replicated demes.
  cDoubleSum m_deme_fit_sd; //!< Mean standard deviation of fitness of organisms within a deme
  cDoubleSum m_deme_gest_sd; //!< Mean standard deviation of gestation of organisms within a deme
  cDoubleSum m_deme_merit_sd; //!< Mean standard deviation of merit of organisms within a deme
  cDoubleSum m_germline_generation; //!< Mean germline generation of replicated germlines
  

	int m_deme_num_repls_treatable; //!< Number of deme replications in treatable demes since last PrintDemeReplicationData.
  cDoubleSum m_deme_gestation_time_treatable; //!< Gestation time for treatable demes - mean age at deme replication.
  cDoubleSum m_deme_births_treatable; //!< Mean number of births in replicated treatable demes.
  cDoubleSum m_deme_merit_treatable; //!< Mean merit of replicated treatable demes.
  cDoubleSum m_deme_generation_treatable; //!< Mean generation of replicated treatable demes.
	cDoubleSum m_deme_density_treatable; //!< Mean density of replicated treatable demes.

	int m_deme_num_repls_untreatable; //!< Number of deme replications in untreatable demes since last PrintDemeReplicationData.
  cDoubleSum m_deme_gestation_time_untreatable; //!< Gestation time for untreatable demes - mean age at deme replication.
  cDoubleSum m_deme_births_untreatable; //!< Mean number of births in replicated untreatable demes.
  cDoubleSum m_deme_merit_untreatable; //!< Mean merit of replicated untreatable demes.
  cDoubleSum m_deme_generation_untreatable; //!< Mean generation of replicated untreatable demes.
	cDoubleSum m_deme_density_untreatable; //!< Mean density of replicated untreatable demes.

	t_founder_map m_deme_founders; //!< Data structure to track the founders of demes.


  // -------- Deme competition support --------
public:
  //! Called immediately prior to deme competition.
  void CompeteDemes(const std::vector<double>& fitness);
  //! Print data regarding deme competition.
  void PrintDemeCompetitionData(const cString& filename);

private:
  std::vector<double> m_deme_fitness; //!< Fitness of each deme during last deme competition.

	// -------- Cell data support --------
public:
	//! Prints the cell data from every cell in the population.
	void PrintCellData(const cString& filename);

	// -------- Opinion support --------
public:
	//! Prints the current opinions of all organisms in the population.
	void PrintCurrentOpinions(const cString& filename);
	//! Prints the average number of organism with set opinions
	void PrintOpinionsSetPerDeme(const cString& filename);

	// -------- Synchronization support --------
public:
	typedef std::vector<int> CellFlashes; //!< Typedef for a list of cell IDs.
	typedef std::map<int, CellFlashes> DemeFlashes; //!< Typedef for cell IDs (in this deme) -> list of cell IDs.
	typedef std::map<int, DemeFlashes> PopulationFlashes; //!< Typedef for deme IDs -> flashes in that deme.
  //! Called immediately after an organism has issued a "flash" to its neighbors.
  void SentFlash(cOrganism& organism);
	//! Retrieve the cell ID -> flash time map.
	const PopulationFlashes& GetFlashTimes() { return m_flash_times; }
  //! Print statistics about synchronization flashes.
  void PrintSynchronizationData(const cString& filename);
  //! Print detailed information regarding synchronization flashes.
  void PrintDetailedSynchronizationData(const cString& filename);
protected:
  int m_flash_count; //!< Number of flashes that have occured since last PrintSynchronizationData.
	PopulationFlashes m_flash_times; //!< For tracking flashes that have occurred throughout this population.

	// -------- Consensus support --------
public:
	struct ConsensusRecord {
		ConsensusRecord(int u, int d, cOrganism::Opinion c, int cell) : update(u), deme_id(d), consensus(c), cell_id(cell) {
		}
		int update;
		int deme_id;
		cOrganism::Opinion consensus;
		int cell_id;
	};

	typedef std::vector<ConsensusRecord> Consensi; //!< Typedef for a map of update -> Consensus records.
	//! Called when a deme reaches consensus.
	void ConsensusReached(const cDeme& deme, cOrganism::Opinion consensus, int cellid);
	//! Print information about demes that have reached consensus.
	void PrintConsensusData(const cString& filename);
	//! Print "simple" (summary) consensus information.
	void PrintSimpleConsensusData(const cString& filename);
protected:
	Consensi m_consensi; //!< Tracks when demes have reached consensus.

// ----------Division of Labor support --------
protected:
	typedef std::deque<double> dblq;
	dblq m_switching;
	dblq m_deme_diversity;
	dblq m_shannon_div;
  dblq m_shannon_div_norm;
	dblq m_num_orgs_perf_reaction;
  dblq m_percent_reproductives;

public:
	void IncDemeReactionDiversityReplicationData(double deme_div, double switch_pen,  \
																							  double shannon_div, double num_orgs_perf_reaction, double per_repro) {
		m_switching.push_back(switch_pen); m_deme_diversity.push_back(deme_div);
		m_shannon_div.push_back(shannon_div);
		m_num_orgs_perf_reaction.push_back(num_orgs_perf_reaction);
    double norm_shan = shannon_div / (log((double)num_orgs_perf_reaction));
    m_shannon_div_norm.push_back(norm_shan);
    m_percent_reproductives.push_back(per_repro);

	}

	void PrintAgePolyethismData(const cString& filename);
	void AgeTaskEvent(int org_id, int task_id, int org_age);
	//! Get number of deme replications
	int GetNumDemeReplications() { return m_total_deme_num_repls; }


protected:
	std::map<int, cDoubleSum> reaction_age_map;


// -------- Reputation support ---------
public:
	// Print statistics about reputation
	void PrintReputationData(const cString& filename);
	void PrintDirectReciprocityData(const cString& filename);
	void IncDonateToDonor() { m_donate_to_donor++; }
	void IncDonateToFacing() { m_donate_to_facing++; }
	void PrintStringMatchData(const cString& filename);
	void AddStringBitsMatchedValue(cString name, int value) { m_string_bits_matched[name].Add(value); }
	void AddTag(int tag, int value) { m_tags[tag] = m_tags[tag] + value; }
	void IncPerfectMatch(int amount) { m_perfect_match.Add(amount); }
	void IncPerfectMatchOrg() { m_perfect_match_org.Add(1); }
	void PrintShadedAltruists(const cString& filename);

protected:
	int m_donate_to_donor;
	int m_donate_to_facing;
	std::map <cString, cDoubleSum> m_string_bits_matched;
	cDoubleSum m_perfect_match;
	cDoubleSum m_perfect_match_org;
	std::map <int, int> m_tags;

	// -------- Deme network support --------
public:
	typedef std::map<std::string, double> network_stats_t; //!< Structure to hold statistics for one network.
	typedef std::map<std::string, cDoubleSum> avg_network_stats_t; //!< Structure to hold average statistics for many networks.

	//! Track named network stats.
	void NetworkTopology(const network_stats_t& ns);

	//! Print network statistics.
	void PrintDemeNetworkData(const cString& filename);

	//! Print the topologies of all demes.
	void PrintDemeNetworkTopology(const cString& filename);

protected:
	avg_network_stats_t m_network_stats; //!< Network statistics.

	// -------- HGT support --------
private:
	cDoubleSum m_hgt_metabolized; //!< Total length of metabolized genome fragments.
	cDoubleSum m_hgt_inserted; //!< Total length of inserted genome fragments.
public:
	//! Called when an organism metabolizes a genome fragment.
	void GenomeFragmentMetabolized(cOrganism* organism, const InstructionSequence& fragment);
	//! Called when an organism inserts a genome fragment.
	void GenomeFragmentInserted(cOrganism* organism, const InstructionSequence& fragment, const cGenomeUtil::substring_match& location);
	//! Print HGT statistics.
	void PrintHGTData(const cString& filename);

	// -------- Multiprocess support --------
private:
	cDoubleSum m_outgoing; //!< Number of outgoing migration events.
	cDoubleSum m_incoming; //!< Number of incoming migration events.

public:
	typedef std::map<std::string, double> profiling_stats_t; //!< Structure to hold average profiling statistics.
	typedef std::map<std::string, cDoubleSum> avg_profiling_stats_t; //!< Structure to hold statistics for one network.

	//! Record information about an organism migrating from this population.
	void OutgoingMigrant(const cOrganism* org);

	//! Record information about an organism migrating into this population.
	void IncomingMigrant(const cOrganism* org);

	//! Print multiprocess data.
	void PrintMultiProcessData(const cString& filename);

	//! Track profiling data.
	void ProfilingData(const profiling_stats_t& pf);

	//! Print profiling data.
	void PrintProfilingData(const cString& filename);

protected:
	avg_profiling_stats_t m_profiling; //!< Profiling statistics.
	
	// -------- Support for organism locations --------
public:
	//! Print organism locations.
	void PrintOrganismLocation(const cString& filename);
  
  
private:
  // Initialization
  void setupProvidedData();
  
  // Helper Methods
  template <class T> Data::PackagePtr packageData(T (cStats::*)() const) const;
  template <class T, class U> Data::PackagePtr packageArgData(T (cStats::*)(U arg) const, U arg) const;
};


inline void cStats::SetBreedTrueCreatures(int in_num_breed_true_creatures)
{
  num_breed_true_creatures = in_num_breed_true_creatures;
}

inline void cStats::SetNumParasites(int in_num_parasites)
{
  num_parasites = in_num_parasites;
}

inline void cStats::SetNumNoBirthCreatures(int in_num_no_birth_creatures)
{
  num_no_birth_creatures = in_num_no_birth_creatures;
}

inline void cStats::SetNumSingleThreadCreatures(int in_num_single_thread_creatures)
{
  num_single_thread_creatures = in_num_single_thread_creatures;
}

inline void cStats::SetNumMultiThreadCreatures(int in_num_multi_thread_creatures)
{
  num_multi_thread_creatures = in_num_multi_thread_creatures;
}

inline void cStats::SetNumModified(int in_num_modified)
{
  num_modified = in_num_modified;
}

#endif
