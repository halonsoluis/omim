#pragma once

#include "drape_frontend/map_shape.hpp"
#include "drape_frontend/shape_view_params.hpp"

#include "geometry/spline.hpp"

namespace df
{

class PathTextShape : public MapShape
{
public:
  PathTextShape(m2::SharedSpline const & spline,
                PathTextViewParams const & params);
  virtual void Draw(dp::RefPointer<dp::Batcher> batcher, dp::RefPointer<dp::TextureManager> textures) const;

private:
  m2::SharedSpline m_spline;
  PathTextViewParams m_params;
};

} // namespace df
