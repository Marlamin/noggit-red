#include <noggit/Selection.h>
#include <noggit/MapChunk.h>
#include <noggit/DBC.h>


void selected_chunk_type::updateDetails(noggit::ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  mcnk_flags const& flags = chunk->header_flags;

  select_info << "MCNK " << chunk->px << ", " << chunk->py << " (" << chunk->py * 16 + chunk->px
    << ") of tile (" << chunk->mt->index.x << " " << chunk->mt->index.z << ")"
    << "\narea ID: " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
    << "\nflags: "
    << (flags.flags.has_mcsh ? "shadows " : "")
    << (flags.flags.impass ? "impassable " : "")
    << (flags.flags.lq_river ? "river " : "")
    << (flags.flags.lq_ocean ? "ocean " : "")
    << (flags.flags.lq_magma ? "lava" : "")
    << (flags.flags.lq_slime ? "slime" : "")
    << "\ntextures used: " << chunk->texture_set->num();

  //! \todo get a list of textures and their flags as well as detail doodads.

  select_info << "\n";

  detail_widget->setText(select_info.str());
}