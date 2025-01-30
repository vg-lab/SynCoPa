//
// Created by gaeqs on 2/06/22.
//

#ifndef SYNCOPA_DATAEXPORTER_H
#define SYNCOPA_DATAEXPORTER_H

#include <nsol/DataSet.h>

namespace syncopa
{

  struct XMLExportParameters
  {
    bool exportMorphologyStatistics = true;
    bool exportAggregatedStatistics = true;
    std::vector< nsol::TAggregation > aggregatedStatistics =
      { nsol::TAggregation::TOTAL };
  };

  /**
   * Exports the dataset as a scene XML.
   * <p>
   * You can define which statistics should be exported
   * modifying the given parameter structure. Statistics
   * will only be exported when the dataset contains them.
   * <p>
   * The resulting XML will have the following structure:
   *
   * @code
   * &lt;scene version=""&gt;
   *   &lt;morphology&gt;
   *     &lt;columns&gt;
   *       &lt;column id=""&gt;
   *         &lt;minicolumn id=""&gt;
   *           &lt;neuron gid="" layer="" morphologicalType="" functionalType=""&gt;
   *             &lt;transform&gt;
   *                 ...
   *             &lt;/transform&gt;
   *             &lt;morphology&gt;
   *               &lt;stats&gt;
   *                 &lt;total&gt;
   *                   &lt;dentritic_volume&gt;...&lt;/dentritic_volume&gt;
   *                   &lt;axon_volume&gt;...&lt;/axon_volume&gt;
   *                   &lt;neuritic_volume&gt;...&lt;/neuritic_volume&gt;
   *                   &lt;soma_volume&gt;...&lt;/soma_volume&gt;
   *                   &lt;volume&gt;...&lt;/volume&gt;
   *                   &lt;dentritic_surface&gt;...&lt;/dentritic_surface&gt;
   *                   &lt;axon_surface&gt;...&lt;/axon_surface&gt;
   *                   &lt;neuritic_surface&gt;...&lt;/neuritic_surface&gt;
   *                   &lt;soma_surface&gt;...&lt;/soma_surface&gt;
   *                   &lt;surface&gt;...&lt;/surface&gt;
   *                   &lt;dentritic_length&gt;...&lt;/dentritic_length&gt;
   *                   &lt;axon_length&gt;...&lt;/axon_length&gt;
   *                   &lt;neuritic_length&gt;...&lt;/neuritic_length&gt;
   *                   &lt;dentritic_bifurcations&gt;...&lt;/dentritic_bifurcations&gt;
   *                   &lt;axon_bifurcations&gt;...&lt;/axon_bifurcations&gt;
   *                   &lt;neuritic_bifurcations&gt;...&lt;/neuritic_bifurcations&gt;
   *                 &lt;/total&gt;
   *                 ...
   *               &lt;/stats&gt;
   *               &lt;soma&gt;
   *                 &lt;surface&gt;...&lt;/surface&gt;
   *                 &lt;volume&gt;...&lt;/volume&gt;
   *               &lt;/soma&gt;
   *               &lt;axon&gt;
   *                 &lt;surface&gt;...&lt;/surface&gt;
   *                 &lt;volume&gt;...&lt;/volume&gt;
   *                 &lt;length&gt;...&lt;/length&gt;
   *                 &lt;bifurcations&gt;...&lt;/bifurcations&gt;
   *               &lt;/axon&gt;
   *               &lt;dendrites&gt;
   *                 &lt;dendrite dentrite_type="" neurite_type=""&gt;
   *                   &lt;surface&gt;...&lt;/surface&gt;
   *                   &lt;volume&gt;...&lt;/volume&gt;
   *                   &lt;length&gt;...&lt;/length&gt;
   *                   &lt;bifurcations&gt;...&lt;/bifurcations&gt;
   *                 &lt;/dendrite&gt;
   *                 ...
   *               &lt;/dendrites&gt;
   *             &lt;/morphology&gt;
   *           &lt;/neuron&gt;
   *           ...
   *         &lt;/minicolumn&gt;
   *         ...
   *       &lt;/column&gt;
   *       ...
   *     &lt;/columns&gt;
   *   &lt;/morphology&gt;
   * &lt;/scene&gt;
   * @endcode
   *
   * @param dataset the dataset where the data is stored.
   * @param out the output stream where the XML will be written into.
   * @param parameters the configuration of the exporter.
   */
  void toXML( const nsol::DataSet* dataset , std::ostream& out ,
              const XMLExportParameters& parameters = XMLExportParameters( ));

  /**
   *
   * Exports the dataset as an aggregated connection CSV.
   * <p>
   * This CSV exports the amount of connections a pre-synaptic
   * neuron has with a post-synaptic neuron, without any other data.
   * <p>
   * The resulting CSV will have the following structure:
   *
   * @code
   * PreNeuronId;PostNeuronId;Weight
   * ...;...;...
   * @endcode
   *
   * You can modify the row and column separators.
   *
   * @param dataset the dataset where the data is stored.
   * @param out the output stream where the CSV will be written into.
   * @param columnSeparator the column separator.
   * @param rowSeparator the row separator.
   */
  void toAggregateCSV( const nsol::DataSet* dataset , std::ostream& out ,
                       const char columnSeparator = ';' , const char rowSeparator = '\n' );

  /**
   *
   * Exports the dataset as a compact connection CSV.
   * <p>
   * This CSV exports a row per synapse, including all its
   * its statistics and parameters. There may be more than
   * one synapse per pair of pre-synaptic and post-synaptic neurons.
   * <p>
   * The resulting CSV will have the following structure:
   *
   * @code
   * SynId;PreNeuronId;PostNeuronId;Delay;Conductance;Utilization;Depression;Facilitation;Efficacy
   * ...;...;...;...;...;...;...;...;...
   * @endcode
   *
   * You can modify the row and column separators.
   *
   * @param dataset the dataset where the data is stored.
   * @param out the output stream where the CSV will be written into.
   * @param columnSeparator the column separator.
   * @param rowSeparator the row separator.
   */
  void toCompactCSV( const nsol::DataSet* dataset , std::ostream& out ,
                     const char columnSeparator = ';' , const char rowSeparator = '\n' );

  /**
   *
   * Exports the dataset as a compact matrix CSV.
   *
   * This CSV exports a row and column per neuron.
   * Each matrix field will have as the value the amount
   * of synapses between pre-synaptic neuron (row) and
   * the post-synaptic neuron (column).
   * <p>
   * You can modify the row and column separators.
   * <p>
   * This CSV format is NOT recommended, as it has a lot of
   * redundancy. Use <i>toAggregateCSV()</i> instead.
   *
   * @param dataset the dataset where the data is stored.
   * @param out the output stream where the CSV will be written into.
   * @param columnSeparator the column separator.
   * @param rowSeparator the row separator.
   */
  void toMatrixCSV( const nsol::DataSet* dataset , std::ostream& out ,
                    const char columnSeparator = ';' , const char rowSeparator = '\n' );

 QJsonObject toHanoiJSON( const nsol::DataSet* dataset );

}


#endif //SYNCOPA_DATAEXPORTER_H
