/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#ifndef PAGMO_ARCHIPELAGO_H
#define PAGMO_ARCHIPELAGO_H

#include <boost/scoped_ptr.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "algorithm/base.h"
#include "config.h"
#include "island.h"
#include "population.h"
#include "problem/base.h"
#include "rng.h"
#include "topology/base.h"
#include "topology/unconnected.h"

namespace pagmo {

/// Archipelago class.
/**
 * @author Francesco Biscani (bluescarni@gmail.com)
 * @author Marek Ruciński (marek.rucinski@gmail.com)
 */
class __PAGMO_VISIBLE archipelago
{
	public:
		/// Island class must have access to internal archipelago methods.
		friend class island;
		/// Internal container of islands.
		typedef std::vector<island> container_type;
		/// Archipelago size type.
		typedef container_type::size_type size_type;
		/// Individual type.
		typedef population::individual_type individual_type;
		/// Distribution type for migrating individuals.
		enum distribution_type
		{
			/// Individuals migrate to one of the island's neighbours.
			/**
			 * The destination will be chosen randomly among all the possible choices.
			 */
			point_to_point = 0,
			/// Individuals migrate to all the island's neighbours.
			/**
			 * Note that in case of highly-connected topologies this options can result in high memory usage.
			 */
			broadcast = 1
		};
		/// Migration direction.
		/**
		 * This parameters controls how islands migrate individuals to/from their neighbours. It is a fine tuning parameter for which the default
		 * value is appropriate for most uses.
		 */
		enum migration_direction
		{
			/// Immigrants flow is initiated by the source island.
			/**
			 * With this strategy, the internal migration database stores for each island the individuals that are meant to migrate
			 * to that island. Before each evolution, an island will check if individuals destined to it are available in the database,
			 * and, in such case will, migrate over incoming individuals before starting evolution.
			 *
			 * After each evolution, the island will place its candidate individuals for emigration in the database slots of the island(s) to which
			 * it connects.
			 */
			source = 0,
			/// Immigrants flow is initiated by the destination island.
			/**
			 * With this strategy, the internal migration database stores for each island its best individuals. Before each evolution,
			 * the island will get migrating individuals from those made available by the islands connecting to it.
			 *
			 * After each evolution, the island will update its list of best individuals in the database.
			 */
			destination = 1
		};
	private:
		// Iterators.
		typedef container_type::iterator iterator;
		typedef container_type::const_iterator const_iterator;
		// Container for migrating individuals. This a hash map containing hash maps as values.
		// Please NOTE carefully: in case of desination migration, item n in the outer hash map is supposed to contain a hash map with a single
		// (n,emigrants vector) pair (in other words, containing redundantly n twice). In case of source migration, item n will contain a map of
		// emigrants from other islands.
		typedef boost::unordered_map<size_type,boost::unordered_map<size_type,std::vector<individual_type> > > migration_map_type;
		// Lock type.
		typedef boost::lock_guard<boost::mutex> lock_type;
		// Migration history item: (n_individuals,orig_island,dest_island) tuple.
		typedef boost::tuple<population::size_type,size_type,size_type> migr_hist_item;
		// Container of migration history: vector of history items.
		typedef std::vector<migr_hist_item> migr_hist_type;
	public:
		archipelago(distribution_type dt = point_to_point, migration_direction md = destination);
		archipelago(const topology::base &t, distribution_type dt = point_to_point, migration_direction md = destination);
		archipelago(const problem::base &p, const algorithm::base &a, int n, int m, const topology::base &t = topology::unconnected(),
			distribution_type dt = point_to_point, migration_direction md = destination);
		archipelago(const archipelago &);
		archipelago &operator=(const archipelago &);
		~archipelago();
		void join() const;
		void push_back(const island &);
		size_type get_size() const;
		std::string human_readable() const;
		bool check_island(const island &) const;
		topology::base_ptr get_topology() const;
		void set_topology(const topology::base &);
		void evolve(int n = 1);
		void evolve_t(int);
		bool busy() const;
		void interrupt();
		std::string dump_migr_history() const;
		void clear_migr_history();
	private:
		void pre_evolution(island &);
		void post_evolution(island &);
		void reset_barrier(const size_type &);
		void build_immigrants_vector(std::vector<individual_type> &, const island &,
			island &, const std::vector<individual_type> &, migr_hist_type &) const;
		void check_migr_attributes() const;
		void sync_island_start() const;
		struct count_if_blocking;
	private:
		// Container of islands.
		container_type				m_container;
		// A barrier used to synchronise the start time of all islands.
		boost::scoped_ptr<boost::barrier>	m_islands_sync_point;
		// Topology.
		topology::base_ptr			m_topology;
		// Distribution type.
		distribution_type			m_dist_type;
		// Migration direction.
		migration_direction			m_migr_dir;
		// Migration container.
		migration_map_type			m_migr_map;
		// Rngs used during migration.
		rng_double				m_drng;
		rng_uint32				m_urng;
		// Migration mutex.
		boost::mutex				m_migr_mutex;
		// Migration history.
		migr_hist_type				m_migr_hist;

};

std::ostream __PAGMO_VISIBLE_FUNC &operator<<(std::ostream &, const archipelago &);

}

#endif