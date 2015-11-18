#pragma once

#include "drape/drape_global.hpp"
#include "drape/binding_info.hpp"
#include "drape/index_buffer_mutator.hpp"
#include "drape/index_storage.hpp"
#include "drape/attribute_buffer_mutator.hpp"

#include "indexer/feature_decl.hpp"

#include "geometry/screenbase.hpp"
#include "geometry/point2d.hpp"
#include "geometry/rect2d.hpp"

#include "base/buffer_vector.hpp"

namespace dp
{

enum OverlayRank
{
  OverlayRank0 = 0,
  OverlayRank1,
  OverlayRank2,

  OverlayRanksCount
};

uint64_t constexpr kPriorityMaskZoomLevel = 0xFF0000000000FFFF;
uint64_t constexpr kPriorityMaskManual    = 0x00FFFFFFFF00FFFF;
uint64_t constexpr kPriorityMaskRank      = 0x0000000000FFFFFF;
uint64_t constexpr kPriorityMaskAll = kPriorityMaskZoomLevel |
                                      kPriorityMaskManual |
                                      kPriorityMaskRank;
class OverlayHandle
{
public:
  typedef vector<m2::RectF> Rects;

  OverlayHandle(FeatureID const & id, dp::Anchor anchor, uint64_t priority);

  virtual ~OverlayHandle() {}

  bool IsVisible() const;
  void SetIsVisible(bool isVisible);

  m2::PointD GetPivot(ScreenBase const & screen) const;

  virtual bool Update(ScreenBase const & /*screen*/) { return true; }
  virtual m2::RectD GetPixelRect(ScreenBase const & screen) const = 0;

  virtual void GetPixelShape(ScreenBase const & screen, Rects & rects) const = 0;

  bool IsIntersect(ScreenBase const & screen, ref_ptr<OverlayHandle> const h) const;

  virtual bool IndexesRequired() const { return true; }
  void * IndexStorage(uint32_t size);
  void GetElementIndexes(ref_ptr<IndexBufferMutator> mutator) const;
  virtual void GetAttributeMutation(ref_ptr<AttributeBufferMutator> mutator,
                                    ScreenBase const & screen) const;

  bool HasDynamicAttributes() const;
  void AddDynamicAttribute(BindingInfo const & binding, uint32_t offset, uint32_t count);

  FeatureID const & GetFeatureID() const;
  uint64_t const & GetPriority() const;

  virtual uint64_t GetPriorityMask() const { return kPriorityMaskAll; }

  virtual bool IsBound() const { return false; }

  int GetOverlayRank() const { return m_overlayRank; }
  void SetOverlayRank(int overlayRank) { m_overlayRank = overlayRank; }

protected:
  FeatureID const m_id;
  dp::Anchor const m_anchor;
  uint64_t const m_priority;

  int m_overlayRank;

  typedef pair<BindingInfo, MutateRegion> TOffsetNode;
  TOffsetNode const & GetOffsetNode(uint8_t bufferID) const;

private:
  bool m_isVisible;

  dp::IndexStorage m_indexes;
  struct LessOffsetNode
  {
    bool operator()(TOffsetNode const & node1, TOffsetNode const & node2) const
    {
      return node1.first.GetID() < node2.first.GetID();
    }
  };

  struct OffsetNodeFinder;

  set<TOffsetNode, LessOffsetNode> m_offsets;
};

class SquareHandle : public OverlayHandle
{
  using TBase = OverlayHandle;

public:
  SquareHandle(FeatureID const & id,
               dp::Anchor anchor,
               m2::PointD const & gbPivot,
               m2::PointD const & pxSize,
               uint64_t priority);

  virtual m2::RectD GetPixelRect(ScreenBase const & screen) const;
  virtual void GetPixelShape(ScreenBase const & screen, Rects & rects) const;

private:
  m2::PointD m_gbPivot;
  m2::PointD m_pxHalfSize;
};

uint64_t CalculateOverlayPriority(int minZoomLevel, uint8_t rank, float depth);

} // namespace dp
