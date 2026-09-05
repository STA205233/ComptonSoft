/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#include "EventTreeIOWithInitialInfo.hh"

namespace comptonsoft
{

EventTreeIOWithInitialInfo::~EventTreeIOWithInitialInfo() = default;

void EventTreeIOWithInitialInfo::set_tree(TTree* tree)
{
  EventTreeIO::set_tree(tree);
  InitialInfoTreeIO::set_tree(tree);
}

void EventTreeIOWithInitialInfo::define_branches()
{
  EventTreeIO::define_branches();
  InitialInfoTreeIO::define_branches();
}

void EventTreeIOWithInitialInfo::set_branch_addresses()
{
  EventTreeIO::set_branch_addresses();
  InitialInfoTreeIO::set_branch_addresses();
}

} /* namespace comptonsoft */
