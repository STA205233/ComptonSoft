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
    // define_parameter("directory", &mod_class::directory_);
    return AS_OK;
  }

  ANLStatus PushFrameInfoToQuickLookDB::mod_initialize()
  {
    get_module_NC(event_collection_module_name_, &event_collection_module_);
    get_module_NC("LoadMetaDataFile", &metadata_file_module_);
    get_module_NC("FrameData", &frame_module_);

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

    if (mongodb_)
    {
      pushFrameInfoToDB();
    }

    return AS_OK;
  }

  void PushFrameInfoToQuickLookDB::pushFrameInfoToDB()
  {
    const XrayEventContainer &events = event_collection_module_->getEvents();
    const flags_t &disabled_pixels = frame_module_->getDisabledPixels();
    const int nx = (frame_module_->NumPixelsX());
    const int ny = (frame_module->NumPixelsY());
    for (size_t i = 0; i < nx; i++)
    {
      for (size_t j = 0; j < ny; j++)
      {
        if (disabled_pixels[i][j] == 1)
        {
          bad_pixel_++;
        }
      }
    }
    bad_pixel_ratio = bad_pixel / nx / ny;
    std::vector<bsoncxx::document::value> documents;

    for (const auto &event : events)
    {
      // documents.push_back(
      //   bsoncxx::builder::stream::document{}
      //   << "analysis_id" << analysis_id_
      //   << "frameID" << event->FrameID()
      //   << "ix" << event->PixelX()
      //   << "iy" << event->PixelY()
      //   << "sumPH" << event->SumPH()
      //   << "centerPH" << event->CenterPH()
      //   << "angle" << event->Angle()
      //   << "weight" << event->Weight()
      //   << "rank" << event->Rank()
      //   << "temperature" << metadata_file_module_->Temperature()
      //   << "capture_time" << bsoncxx::types::b_date(metadata_file_module_->CaptureTime())
      //   << "filename" << metadata_file_module_->Filename()
      //   << bsoncxx::builder::stream::finalize
      //   );
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
                   //  << "DataSize" << metadata_file_module_->Data_Size()
                   << "Capture_time" << bsoncxx::types::b_date(metadata_file_module_->CaptureTime())
                   << "Filename" << metadata_file_module_->Filename()
                   << "Count_rate" << count_
                   << "Whole_count" << whole_count_
                   << "Bad_pixel_ratio" << Bad_pixel_ratio_
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

} /* namespace comptonsoft */
