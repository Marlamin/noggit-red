// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

layout (std140) uniform lighting
{
  vec4 DiffuseColor_FogStart;
  vec4 AmbientColor_FogEnd;
  vec4 FogColor_FogOn;
  vec4 LightDir_FogRate;
  vec4 OceanColorLight;
  vec4 OceanColorDark;
  vec4 RiverColorLight;
  vec4 RiverColorDark;
};

layout (std140) uniform overlay_params
{
  int draw_shadows;
  int draw_lines;
  int draw_hole_lines;
  int draw_areaid_overlay;
  int draw_terrain_height_contour;
  int draw_wireframe;
  int wireframe_type;
  float wireframe_radius;
  float wireframe_width;
  int draw_impass_overlay;
  int draw_paintability_overlay;
  int draw_selection_overlay;
  vec4 wireframe_color;
  int draw_impassible_climb;
  int climb_use_output_angle;
  int climb_use_smooth_interpolation;
  float climb_value;
  int draw_vertex_color;
  int padding[3];
};

struct ChunkInstanceData
{
  ivec4 ChunkTextureSamplers;
  ivec4 ChunkTextureArrayIDs;
  ivec4 ChunkHoles_DrawImpass_TexLayerCount_CantPaint;
  ivec4 ChunkTexDoAnim;
  ivec4 ChunkTexAnimSpeed;
  ivec4 AreaIDColor_Pad2_DrawSelection;
  ivec4 ChunkXZ_TileXZ;
  ivec4 ChunkTexAnimDir;

  // Mists Heightmapping

  ivec4 ChunkHeightTextureSamplers;
  ivec4 ChunkTextureUVScale;
  vec4 ChunkTextureHeightScale;
  vec4 ChunkTextureHeightOffset;

};

layout (std140) uniform chunk_instances
{
  ChunkInstanceData instances[256];
};

uniform sampler2DArray shadowmap;
uniform sampler2DArray alphamap;
uniform sampler2D stamp_brush;
uniform sampler2DArray textures[11];
uniform vec3 camera;

uniform int draw_cursor_circle;
uniform vec3 cursor_position;
uniform float cursorRotation;
uniform float outer_cursor_radius;
uniform float inner_cursor_ratio;
uniform vec4 cursor_color;

in vec3 vary_position;
in vec2 vary_texcoord;
in vec2 vary_t0_uv;
in vec2 vary_t1_uv;
in vec2 vary_t2_uv;
in vec2 vary_t3_uv;
in vec3 vary_normal;
in vec3 vary_mccv;
flat in int instanceID;
flat in vec3 triangle_normal;

out vec4 out_color;

const float TILESIZE  = 533.33333;
const float CHUNKSIZE = TILESIZE / 16.0;
const float HOLESIZE  = CHUNKSIZE * 0.25;
const float UNITSIZE = HOLESIZE * 0.5;
const float PI = 3.14159265358979323846;

vec3 random_color(float areaID)
{
  float r = fract(sin(dot(vec2(areaID), vec2(12.9898, 78.233))) * 43758.5453);
  float g = fract(sin(dot(vec2(areaID), vec2(11.5591, 70.233))) * 43569.5451);
  float b = fract(sin(dot(vec2(areaID), vec2(13.1234, 76.234))) * 43765.5452);

  return vec3(r, g, b);
}

vec4 get_tex_color(vec2 tex_coord, int tex_sampler, int array_index)
{
  if (tex_sampler == 0)
  {
    return texture(textures[0], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 1)
  {
    return texture(textures[1], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 2)
  {
    return texture(textures[2], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 3)
  {
    return texture(textures[3], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 4)
  {
    return texture(textures[4], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 5)
  {
    return texture(textures[5], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 6)
  {
    return texture(textures[6], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 7)
  {
    return texture(textures[7], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 8)
  {
    return texture(textures[8], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 9)
  {
    return texture(textures[9], vec3(tex_coord, array_index)).rgba;
  }
  else if (tex_sampler == 10)
  {
    return texture(textures[10], vec3(tex_coord, array_index)).rgba;
  }

  return vec4(0);


  /*
  // This should be more compliant to the GLSL standard, but seems to be slower :(
  vec2 uvDx = dFdx(tex_coord);
  vec2 uvDy = dFdy(tex_coord);
  vec3 uv = vec3(tex_coord, array_index);
  
  switch(tex_sampler) 
  {
    case 0:
      return textureGrad(textures[0], uv, uvDx, uvDy);
    case 1:
      return textureGrad(textures[1], uv, uvDx, uvDy);
    case 2:
      return textureGrad(textures[2], uv, uvDx, uvDy);
    case 3:
      return textureGrad(textures[3], uv, uvDx, uvDy);
    case 4:
      return textureGrad(textures[4], uv, uvDx, uvDy);
    case 5:
      return textureGrad(textures[5], uv, uvDx, uvDy);
    case 6:
      return textureGrad(textures[6], uv, uvDx, uvDy);
    case 7:
      return textureGrad(textures[7], uv, uvDx, uvDy);
    case 8:
      return textureGrad(textures[8], uv, uvDx, uvDy);
    case 9:
      return textureGrad(textures[9], uv, uvDx, uvDy);
    case 10:
      return textureGrad(textures[10], uv, uvDx, uvDy);
    default:
      return vec4(0.0);
  }

  */

}

/*vec2 getAdjustedUV(vec2 uv, float scale) {
    // Calculate the world position in the XZ plane
    vec2 worldPosXZ = vary_position.xz;

    // Scale the world position by the desired scale factor
    vec2 scaledWorldPos = (worldPosXZ / 16.0f) / (scale);

    // Calculate the new UV coordinates by adding the original UV and scaled world position
    vec2 scaledUV = (uv / scale) + scaledWorldPos;

    return scaledUV;
}*/

const int numChunks = 16;
const int numTiles = 64;

vec2 getAdjustedUV(vec2 uv, int textureScale) {

    vec2 worldOffset = (numChunks * instances[instanceID].ChunkXZ_TileXZ.zw) + 
                                instances[instanceID].ChunkXZ_TileXZ.xy;

    // Scale the UV coordinates. Wow Interprets texture scaling this way.
    vec2 combinedUV = fract((uv + worldOffset) / ((1 << textureScale) / 8.0f));

    return combinedUV;

}

vec4 texture_blend()
{
  vec3 alpha = texture(alphamap, vec3(vary_texcoord / 8.0, instanceID)).rgb;

  int layer_count = instances[instanceID].ChunkHoles_DrawImpass_TexLayerCount_CantPaint.b;

  alpha.r = mix(alpha.r, 0.0, float(layer_count < 2));
  alpha.g = mix(alpha.g, 0.0, float(layer_count < 3));
  alpha.b = mix(alpha.b, 0.0, float(layer_count < 4));

  float a0 = alpha.r;
  float a1 = alpha.g;
  float a2 = alpha.b;

  vec2 worldScaled_vary_t0_uv = getAdjustedUV(vary_t0_uv/8.0, instances[instanceID].ChunkTextureUVScale.x);
  vec2 worldScaled_vary_t1_uv = getAdjustedUV(vary_t1_uv/8.0, instances[instanceID].ChunkTextureUVScale.y);
  vec2 worldScaled_vary_t2_uv = getAdjustedUV(vary_t2_uv/8.0, instances[instanceID].ChunkTextureUVScale.z);
  vec2 worldScaled_vary_t3_uv = getAdjustedUV(vary_t3_uv/8.0, instances[instanceID].ChunkTextureUVScale.w);


  vec4 t0 = get_tex_color(worldScaled_vary_t0_uv, instances[instanceID].ChunkTextureSamplers.x, abs(instances[instanceID].ChunkTextureArrayIDs.x));
  t0.a = mix(t0.a, 0.f, int(instances[instanceID].ChunkTextureArrayIDs.x < 0));

  vec4 t1 = get_tex_color(worldScaled_vary_t1_uv, instances[instanceID].ChunkTextureSamplers.y, abs(instances[instanceID].ChunkTextureArrayIDs.y));
  //vec4 t1 = get_tex_color(vary_t1_uv / 5 , instances[instanceID].ChunkTextureSamplers.y, abs(instances[instanceID].ChunkTextureArrayIDs.y));
  t1.a = mix(t1.a, 0.f, int(instances[instanceID].ChunkTextureArrayIDs.y < 0));

  vec4 t2 = get_tex_color(worldScaled_vary_t2_uv, instances[instanceID].ChunkTextureSamplers.z, abs(instances[instanceID].ChunkTextureArrayIDs.z));
  t2.a = mix(t2.a, 0.f, int(instances[instanceID].ChunkTextureArrayIDs.z < 0));

  vec4 t3 =get_tex_color(worldScaled_vary_t3_uv, instances[instanceID].ChunkTextureSamplers.w, abs(instances[instanceID].ChunkTextureArrayIDs.w));
  t3.a = mix(t3.a, 0.f, int(instances[instanceID].ChunkTextureArrayIDs.w < 0));

  return vec4 (t0 * (1.0 - (a0 + a1 + a2)) + t1 * a0 + t2 * a1 + t3 * a2);
}

float contour_alpha(float unit_size, float pos, float line_width)
{
  float f = abs(fract((pos + unit_size*0.5) / unit_size) - 0.5);
  float df = abs(line_width / unit_size);
  return smoothstep(0.0, df, f);
}

float contour_alpha(float unit_size, vec2 pos, vec2 line_width)
{
  return 1.0 - min( contour_alpha(unit_size, pos.x, line_width.x)
                  , contour_alpha(unit_size, pos.y, line_width.y)
                  );
}

void main()
{
  float dist_from_camera = distance(camera, vary_position);

  vec3 fw = fwidth(vary_position.xyz);

  // calc world lighting
  vec3 currColor;
  vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
  vec3 accumlatedLight = vec3(1.0, 1.0, 1.0);

  vec3 normalized_normal = normalize(vary_normal);
  float nDotL = clamp(dot(normalized_normal, -normalize(LightDir_FogRate.xyz)), 0.0, 1.0);

  vec3 skyColor = (AmbientColor_FogEnd.xyz * 1.10000002);
  vec3 groundColor = (AmbientColor_FogEnd.xyz * 0.699999988);

  currColor = mix(groundColor, skyColor, 0.5 + (0.5 * nDotL));
  lDiffuse = DiffuseColor_FogStart.xyz * nDotL;

  vec3 reflection = normalize(normalized_normal - (-LightDir_FogRate.xyz));
  float specularFactor = max(dot(reflection, normalize(camera - vary_position)), 0.0);

  // blend textures
  out_color = mix(vec4(1.0, 1.0, 1.0, 0.0), texture_blend(), int(instances[instanceID].ChunkHoles_DrawImpass_TexLayerCount_CantPaint.b > 0));

  vec3 spc = out_color.a * out_color.rgb * pow(specularFactor, 8);
  out_color.a = 1.0;

  // apply vertex color
  if (draw_vertex_color != 0)
  {
    out_color.rgb *= vary_mccv;
  }

  // apply world lighting
  out_color.rgb = clamp(out_color.rgb * (currColor + lDiffuse + spc), 0.0, 1.0);

  // apply overlays
  if(draw_paintability_overlay != 0 && instances[instanceID].ChunkHoles_DrawImpass_TexLayerCount_CantPaint.a != 0)
  {
    out_color *= vec4(1.0, 0.0, 0.0, 1.0);
  }

  if(draw_areaid_overlay != 0)
  {
    out_color.rgb = out_color.rgb * 0.3 + random_color(instances[instanceID].AreaIDColor_Pad2_DrawSelection.r);
  }

  if(draw_impass_overlay != 0 && instances[instanceID].ChunkHoles_DrawImpass_TexLayerCount_CantPaint.g != 0)
  {
    out_color.rgb = mix(vec3(1.0), out_color.rgb, 0.5);
  }

  if(draw_selection_overlay != 0 && instances[instanceID].AreaIDColor_Pad2_DrawSelection.a != 0)
  {
   out_color.rgb = mix(vec3(1.0), out_color.rgb, 0.5);
  }

  if (draw_shadows != 0)
  {
    float shadow_alpha = texture(shadowmap, vec3(vary_texcoord / 8.0, instanceID)).r;
    out_color = vec4 (out_color.rgb * (1.0 - shadow_alpha), 1.0);
  }

  if (draw_terrain_height_contour != 0)
  {
    out_color = vec4(out_color.rgb * contour_alpha(4.0, vary_position.y+0.1, fw.y), 1.0);
  }

  bool lines_drawn = false;
  if(draw_lines != 0)
  {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    color.a = contour_alpha(TILESIZE, vary_position.xz, fw.xz * 1.5);
    color.g = color.a > 0.0 ? 0.8 : 0.0;

    if(color.a == 0.0)
    {
      color.a = contour_alpha(CHUNKSIZE, vary_position.xz, fw.xz);
      color.r = color.a > 0.0 ? 0.8 : 0.0;
    }
    if(draw_hole_lines != 0 && color.a == 0.0)
    {
      color.a = contour_alpha(HOLESIZE, vary_position.xz, fw.xz * 0.75);
      color.b = 0.8;
    }

    lines_drawn = color.a > 0.0;
    out_color.rgb = mix(out_color.rgb, color.rgb, color.a);
  }


  if(FogColor_FogOn.w != 0)
  {
    float start = AmbientColor_FogEnd.w * DiffuseColor_FogStart.w; // 0

    vec3 fogParams;
    fogParams.x = -(1.0 / (AmbientColor_FogEnd.w - start)); // - 1 / 338
    fogParams.y = (1.0 / (AmbientColor_FogEnd.w - start)) * AmbientColor_FogEnd.w; // 1 / 338 * 338
    fogParams.z = LightDir_FogRate.w; // 2.7

    float f1 = (dist_from_camera * fogParams.x) + fogParams.y; // 1.0029
    float f2 = max(f1, 0.0);
    float f3 = pow(f2, fogParams.z);
    float f4 = min(f3, 1.0);

    float fogFactor = 1.0 - f4;

    float alpha = clamp((dist_from_camera - start) / (AmbientColor_FogEnd.w - start), 0.0, 1.0);

    out_color.rgb = mix(out_color.rgb, FogColor_FogOn.rgb, fogFactor);
  }

  if(draw_wireframe != 0 && !lines_drawn)
  {
    // true by default => type 0
	  bool draw_wire = true;
      float real_wireframe_radius = max(outer_cursor_radius * wireframe_radius, 2.0 * UNITSIZE);

	  if(wireframe_type == 1)
	  {
		  draw_wire = (length(vary_position.xz - cursor_position.xz) < real_wireframe_radius);
	  }

	  if(draw_wire)
	  {
		  float alpha = contour_alpha(UNITSIZE, vary_position.xz, fw.xz * wireframe_width);
		  float xmod = mod(vary_position.x, UNITSIZE);
		  float zmod = mod(vary_position.z, UNITSIZE);
		  float d = length(fw.xz) * wireframe_width;
		  float diff = min( min(abs(xmod - zmod), abs(xmod - UNITSIZE + zmod))
                      , min(abs(zmod - xmod), abs(zmod + UNITSIZE - zmod))
                      );

		  alpha = max(alpha, 1.0 - smoothstep(0.0, d, diff));
          out_color.rgb = mix(out_color.rgb, wireframe_color.rgb, wireframe_color.a *alpha);
	  }
  }

  if (draw_impassible_climb != 0)
  {
      vec4 color = vec4(out_color.r, out_color.g, out_color.b, 0.5);
      vec3 use_normal;

      if (climb_use_smooth_interpolation != 0)
      {
          use_normal = vary_normal;
      }
      else
      {
          use_normal = triangle_normal;
      }

      float d1 = use_normal.y;
      float d2 = sqrt(use_normal.x * use_normal.x +
                      use_normal.y * use_normal.y +
                      use_normal.z * use_normal.z);

      if (d2 > 0.0)
      {
          float normal_angle = acos(d1/d2);

          if (climb_use_output_angle != 0)
          {
              color.r = normal_angle;
          }
          else
          {
              if (normal_angle > climb_value)
              {
                  color.r = 1.0;
              }
          }
      }


      out_color.rgb = mix(out_color.rgb, color.rgb, color.a);
  }

  if(draw_cursor_circle == 1)
  {
    float diff = length(vary_position.xz - cursor_position.xz);
    diff = min(abs(diff - outer_cursor_radius), abs(diff - outer_cursor_radius * inner_cursor_ratio));
    float alpha = smoothstep(0.0, length(fw.xz), diff);

    out_color.rgb = mix(cursor_color.rgb, out_color.rgb, alpha);
  }
  else if(draw_cursor_circle == 2)
  {
    float angle = cursorRotation * 2.0 * PI;
    vec2 topleft = cursor_position.xz;
    topleft.x -= outer_cursor_radius;
    topleft.y -= outer_cursor_radius;
    vec2 texcoord = (vary_position.xz - topleft) / outer_cursor_radius * 0.5 - 0.5;
    vec2 rotatedTexcoord;
    rotatedTexcoord.x = texcoord.x * cos(angle) + texcoord.y * sin(angle) + 0.5;
    rotatedTexcoord.y = texcoord.y * cos(angle) - texcoord.x * sin(angle) + 0.5;
    /*out_color.rgb = mix(out_color.rgb, texture(stampBrush, rotatedTexcoord).rgb
    , 1.0 * (int(length(vary_position.xz - cursor_position.xz) / outer_cursor_radius < 1.0))
    * (1.0 - length(vary_position.xz - cursor_position.xz) / outer_cursor_radius));*/
    out_color.rgb = mix(out_color.rgb, cursor_color.rgb, texture(stamp_brush, rotatedTexcoord).r
    * int(abs(vary_position.x - cursor_position.x) <= outer_cursor_radius
    && abs(vary_position.z - cursor_position.z) <= outer_cursor_radius));
    /*vec2 posRel = vary_position.xz - cursor_position.xz;
    float pos_x = posRel.x * sin(angle) - posRel.y * cos(angle);
    float pos_z = posRel.y * sin(angle) + posRel.x * cos(angle);
    float diff_x = abs(pos_x);
    float diff_z = abs(pos_z);
    float inner_radius = outer_cursor_radius * inner_cursor_ratio;
    float d = length(fw);
    float alpha = 1.0 * (1 - int((diff_x < outer_cursor_radius && diff_z < outer_cursor_radius
    && (outer_cursor_radius - diff_x <= d || outer_cursor_radius - diff_z <= d)) || (diff_x < inner_radius
    && diff_z < inner_radius && (inner_radius - diff_x <= d || inner_radius - diff_z <= d))));
    out_color.rgb = mix(cursor_color.rgb, out_color.rgb, alpha);*/
  }

}
