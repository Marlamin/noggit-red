#include <noggit/Selection.h>
#include <noggit/MapChunk.h>
#include <noggit/DBC.h>


void selected_chunk_type::updateDetails(Noggit::Ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  mcnk_flags const& flags = chunk->header_flags;

  select_info << "<b>Chunk</b> (" << chunk->px << ", " << chunk->py << ") flat index: (" << chunk->py * 16 + chunk->px
    << ") of <b>tile</b> (" << chunk->mt->index.x << " , " << chunk->mt->index.z << ")"
    << "<br><b>area ID:</b> " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
    << "<br><b>flags</b>: "
    << (flags.flags.has_mcsh ? "<br>shadows " : "")
    << (flags.flags.impass ? "<br>impassable " : "")
    << (flags.flags.lq_river ? "<br>river " : "")
    << (flags.flags.lq_ocean ? "<br>ocean " : "")
    << (flags.flags.lq_magma ? "<br>lava" : "")
    << (flags.flags.lq_slime ? "<br>slime" : "")
    << "<br><b>textures used:</b> " << chunk->texture_set->num()
    << "<br><b>textures:</b><span>";

  unsigned counter = 0;
  for (auto& tex : *(chunk->texture_set->getTextures()))
  {
    bool stuck = !tex->finishedLoading();
    bool error = tex->finishedLoading() && !tex->is_uploaded();

    select_info << "<br> ";

    if (stuck)
      select_info << "<font color=\"Orange\">";

    if (error)
      select_info << "<font color=\"Red\">";

    select_info << "<b>" << (counter + 1) << ":</b> " << tex->file_key().stringRepr();

    if (stuck || error)
      select_info << "</font>";
  }

  //! \todo get a list of textures and their flags as well as detail doodads.

  select_info << "</span><br>";

  detail_widget->setText(select_info.str());
}