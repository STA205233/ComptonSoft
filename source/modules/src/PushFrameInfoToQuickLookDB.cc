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

/*
This is for 2022 SPring-8 experiment.
*/

#include "PushFrameInfoToQuickLookDB.hh"

#include <vector>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include "hsquicklook/MongoDBClient.hh"
#include "hsquicklook/DocumentBuilder.hh"

using namespace anlnext;

namespace comptonsoft
{

  PushFrameInfoToQuickLookDB::PushFrameInfoToQuickLookDB()
      : event_collection_module_name_("XrayEventCollection"),
        collection_("analysis"), document_("images")
  {
  }

  ANLStatus PushFrameInfoToQuickLookDB::mod_define()
  {
    define_parameter("event_collection_module_name", &mod_class::event_collection_module_name_);
    define_parameter("collection", &mod_class::collection_);
    define_parameter("period", &mod_class::period_);
    define_parameter("phase", &mod_class::phase_);
    define_parameter("document", &mod_class::document_);
    return AS_OK;
  }

  ANLStatus PushFrameInfoToQuickLookDB::mod_initialize()
  {
    if (event_collection_module_name != "Dark")
    {
      get_module_NC(event_collection_module_name_, &event_collection_module_);
    }
    get_module_NC("LoadMetaDataFile", &metadata_file_module_);

    if (exist_module("MongoDBClient"))
    {
      get_module_NC("MongoDBClient", &mongodb_);
      mongodb_->createCappedCollection(collection_, 1 * 1024 * 1024);
    }

    return AS_OK;
  }

  ANLStatus PushFrameInfoToQuickLookDB::mod_analyze()
  {
    if (get_loop_index() % period_ != phase_)
    {
      return AS_OK;
    }

    if (mongodb_ && (event_collection_module_name_ != "Dark"))
    {
      pushFrameInfoToDB();
    }
    else if (mongodb_ && (event_collection_module_name_ == "Dark"))
    {
      pushFrameInfoToDBForDark();
    }
    return AS_OK;
  }

  void PushFrameInfoToQuickLookDB::pushFrameInfoToDB()
  {
    const XrayEventContainer &events = event_collection_module_->getEvents();

    for (const auto &event : events)
    {
      count_++;
      whole_count_++;
    }

    hsquicklook::DocumentBuilder builder("Detector", document_);
    const std::time_t t = std::time(nullptr);
    const int64_t ti = static_cast<int64_t>(t) * 64;
    builder.setTI(ti);
    builder.setTimeNow();

    std::string section_name = "Metadata";
    auto section = bsoncxx::builder::stream::document{}
                   << "Loop_counter" << metadata_file_module_->Loop_Counter()
                   << "Capture_time" << bsoncxx::types::b_date(metadata_file_module_->CaptureTime())
                   << "Filename" << metadata_file_module_->Filename()
                   << "Count_rate" << count_
                   << "Whole_count" << whole_count_
                   << bsoncxx::builder::stream::finalize;
    builder.addSection(section_name, section);

    section_name = "Temperature";
    section = bsoncxx::builder::stream::document{}
              << "Temperature" << metadata_file_module_->Temperature()
              << bsoncxx::builder::stream::finalize;
    builder.addSection(section_name, section);

    count_ = 0;

    auto document = builder.generate();
    mongodb_->push(collection_, document);
  }

  void PushFrameInfoToQuickLookDB::pushFrameInfoToDBForDark()
  {
    hsquicklook::DocumentBuilder builder("Detector", document_);
    const std::time_t t = std::time(nullptr);
    const int64_t ti = static_cast<int64_t>(t) * 64;
    builder.setTI(ti);
    builder.setTimeNow();

    std::string section_name = "Metadata";
    auto section = bsoncxx::builder::stream::document{}
                   << "Loop_counter" << metadata_file_module_->Loop_Counter()
                   << "Capture_time" << bsoncxx::types::b_date(metadata_file_module_->CaptureTime())
                   << "Filename" << metadata_file_module_->Filename()
                   << bsoncxx::builder::stream::finalize;
    builder.addSection(section_name, section);

    section_name = "Temperature";
    section = bsoncxx::builder::stream::document{}
              << "Temperature" << metadata_file_module_->Temperature()
              << bsoncxx::builder::stream::finalize;
    builder.addSection(section_name, section);

    auto document = builder.generate();
    mongodb_->push(collection_, document);
  }

} /* namespace comptonsoft */
