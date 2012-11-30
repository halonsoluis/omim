#pragma once

#include "../std/shared_ptr.hpp"
#include "../std/scoped_ptr.hpp"
#include "../std/map.hpp"
#include "../std/string.hpp"
#include "../std/list.hpp"

#include "../base/mutex.hpp"
#include "../base/resource_pool.hpp"

#include "opengl/storage.hpp"
#include "opengl/program_manager.hpp"
#include "glyph_cache.hpp"
#include "data_formats.hpp"

namespace graphics
{
  class Skin;

  namespace gl
  {
    class BaseTexture;
    class Storage;
    class ProgramManager;
  }

  struct GlyphInfo;

  struct TTextureFactory : BasePoolElemFactory
  {
    size_t m_w;
    size_t m_h;
    graphics::DataFormat m_format;
    TTextureFactory(size_t w, size_t h, graphics::DataFormat format, char const * resName, size_t batchSize);
    shared_ptr<gl::BaseTexture> const Create();
  };

  struct TStorageFactory : BasePoolElemFactory
  {
    size_t m_vbSize;
    size_t m_ibSize;
    bool m_useSingleThreadedOGL;
    TStorageFactory(size_t vbSize, size_t ibSize, bool useSingleThreadedOGL, char const * resName, size_t batchSize);
    gl::Storage const Create();
    void BeforeMerge(gl::Storage const & e);
  };

  /// ---- Texture Pools ----

  /// Basic texture pool traits
  typedef BasePoolTraits<shared_ptr<gl::BaseTexture>, TTextureFactory> TBaseTexturePoolTraits;

  /// Fixed-Size texture pool
  typedef FixedSizePoolTraits<TTextureFactory, TBaseTexturePoolTraits > TFixedSizeTexturePoolTraits;
  typedef ResourcePoolImpl<TFixedSizeTexturePoolTraits> TFixedSizeTexturePoolImpl;

  /// On-Demand multi-threaded texture pool

  typedef AllocateOnDemandMultiThreadedPoolTraits<TTextureFactory, TBaseTexturePoolTraits> TOnDemandMultiThreadedTexturePoolTraits;
  typedef ResourcePoolImpl<TOnDemandMultiThreadedTexturePoolTraits> TOnDemandMultiThreadedTexturePoolImpl;

  /// On-Demand single-threaded texture pool
  /// (with postponed resource allocation)
  typedef AllocateOnDemandSingleThreadedPoolTraits<TTextureFactory, TBaseTexturePoolTraits> TOnDemandSingleThreadedTexturePoolTraits;
  typedef ResourcePoolImpl<TOnDemandSingleThreadedTexturePoolTraits> TOnDemandSingleThreadedTexturePoolImpl;

  /// Interface for texture pool
  typedef ResourcePool<shared_ptr<gl::BaseTexture> > TTexturePool;

  /// ---- Storage Pools ----

  /// Basic storage traits
  typedef BasePoolTraits<gl::Storage, TStorageFactory> TBaseStoragePoolTraits;

  /// Fixed-Size mergeable storage pool
  typedef SeparateFreePoolTraits<TStorageFactory, TBaseStoragePoolTraits> TSeparateFreeStoragePoolTraits;
  typedef FixedSizePoolTraits<TStorageFactory, TSeparateFreeStoragePoolTraits> TFixedSizeMergeableStoragePoolTraits;
  typedef ResourcePoolImpl<TFixedSizeMergeableStoragePoolTraits> TFixedSizeMergeableStoragePoolImpl;

  /// Fixed-Size non-mergeable storage pool
  typedef FixedSizePoolTraits<TStorageFactory, TBaseStoragePoolTraits> TFixedSizeNonMergeableStoragePoolTraits;
  typedef ResourcePoolImpl<TFixedSizeNonMergeableStoragePoolTraits> TFixedSizeNonMergeableStoragePoolImpl;

  /// On-Demand single-threaded storage pool
  /// (with postponed resource allocation and separate list of freed resources)
  typedef AllocateOnDemandSingleThreadedPoolTraits<TStorageFactory, TSeparateFreeStoragePoolTraits> TOnDemandSingleThreadedStoragePoolTraits;
  typedef ResourcePoolImpl<TOnDemandSingleThreadedStoragePoolTraits> TOnDemandSingleThreadedStoragePoolImpl;

  /// On-Demand multi-threaded storage pool
  typedef AllocateOnDemandMultiThreadedPoolTraits<TStorageFactory, TBaseStoragePoolTraits> TOnDemandMultiThreadedStoragePoolTraits;
  typedef ResourcePoolImpl<TOnDemandMultiThreadedStoragePoolTraits> TOnDemandMultiThreadedStoragePoolImpl;

  /// Interface for storage pool
  typedef ResourcePool<gl::Storage> TStoragePool;

  class ResourceManager
  {
  public:

    struct StoragePoolParams
    {
      size_t m_vbSize;
      size_t m_vertexSize;
      size_t m_ibSize;
      size_t m_indexSize;
      size_t m_storagesCount;

      bool m_isFixedBufferSize;
      bool m_isFixedBufferCount;

      int m_scalePriority;
      double m_scaleFactor;

      string m_poolName;

      bool m_isDebugging;
      bool m_allocateOnDemand;

      StoragePoolParams(size_t vbSize,
                        size_t vertexSize,
                        size_t ibSize,
                        size_t indexSize,
                        size_t storagesCount,
                        bool isFixedBufferSize,
                        bool isFixedBufferCount,
                        int scalePriority,
                        string const & poolName,
                        bool isDebugging,
                        bool allocateOnDemand);

      StoragePoolParams(string const & poolName);

      bool isFixed() const;
      bool isValid() const;
      void scaleMemoryUsage(double k);
      void distributeFreeMemory(int freeVideoMemory);
      size_t memoryUsage() const;
    };

    struct TexturePoolParams
    {
      size_t m_texWidth;
      size_t m_texHeight;
      size_t m_texCount;
      graphics::DataFormat m_format;

      bool m_isWidthFixed;
      bool m_isHeightFixed;
      bool m_isCountFixed;

      int m_scalePriority;
      double m_scaleFactor;

      string m_poolName;

      bool m_isDebugging;
      bool m_allocateOnDemand;

      TexturePoolParams(size_t texWidth,
                        size_t texHeight,
                        size_t texCount,
                        graphics::DataFormat format,
                        bool isWidthFixed,
                        bool isHeightFixed,
                        bool isCountFixed,
                        int scalePriority,
                        string const & poolName,
                        bool isDebugging,
                        bool allocateOnDemand);

      TexturePoolParams(string const & poolName);

      bool isFixed() const;
      bool isValid() const;
      void scaleMemoryUsage(double k);
      void distributeFreeMemory(int freeVideoMemory);
      size_t memoryUsage() const;
    };

    struct GlyphCacheParams
    {
      string m_unicodeBlockFile;
      string m_whiteListFile;
      string m_blackListFile;

      size_t m_glyphCacheMemoryLimit;


      GlyphCacheParams();
      GlyphCacheParams(string const & unicodeBlockFile,
                       string const & whiteListFile,
                       string const & blackListFile,
                       size_t glyphCacheMemoryLimit);
    };

    struct Params
    {
    private:

      string m_vendorName;
      string m_rendererName;

      /// check non-strict matching upon vendorName and rendererName
      bool isGPU(char const * vendorName, char const * rendererName, bool strictMatch) const;

    public:

      DataFormat m_rtFormat;
      DataFormat m_texFormat;
      DataFormat m_texRtFormat;
      bool m_useSingleThreadedOGL;
      bool m_useReadPixelsToSynchronize;

      size_t m_videoMemoryLimit;

      /// storages params

      StoragePoolParams m_primaryStoragesParams;
      StoragePoolParams m_smallStoragesParams;
      StoragePoolParams m_blitStoragesParams;
      StoragePoolParams m_multiBlitStoragesParams;
      StoragePoolParams m_guiThreadStoragesParams;

      /// textures params

      TexturePoolParams m_primaryTexturesParams;
      TexturePoolParams m_fontTexturesParams;
      TexturePoolParams m_renderTargetTexturesParams;
      TexturePoolParams m_styleCacheTexturesParams;
      TexturePoolParams m_guiThreadTexturesParams;

      /// glyph caches params

      GlyphCacheParams m_glyphCacheParams;

      unsigned m_renderThreadsCount;
      unsigned m_threadSlotsCount;

      Params();

      void distributeFreeMemory(int freeVideoMemory);
      void checkDeviceCaps();
      void fitIntoLimits();
      int memoryUsage() const;
      int fixedMemoryUsage() const;
      void initScaleWeights();
    };

  private:

    typedef map<string, shared_ptr<gl::BaseTexture> > TStaticTextures;

    TStaticTextures m_staticTextures;

    threads::Mutex m_mutex;

    scoped_ptr<TTexturePool> m_primaryTextures;
    scoped_ptr<TTexturePool> m_fontTextures;
    scoped_ptr<TTexturePool> m_styleCacheTextures;
    scoped_ptr<TTexturePool> m_renderTargets;
    scoped_ptr<TTexturePool> m_guiThreadTextures;

    scoped_ptr<TStoragePool> m_primaryStorages;
    scoped_ptr<TStoragePool> m_smallStorages;
    scoped_ptr<TStoragePool> m_blitStorages;
    scoped_ptr<TStoragePool> m_multiBlitStorages;
    scoped_ptr<TStoragePool> m_guiThreadStorages;

    struct ThreadSlot
    {
      shared_ptr<gl::ProgramManager> m_programManager;
      shared_ptr<GlyphCache> m_glyphCache;
    };

    vector<ThreadSlot> m_threadSlots;

    Params m_params;

  public:

    ResourceManager(Params const & p);

    void initThreadSlots(Params const & p);

    void initStoragePool(StoragePoolParams const & p, scoped_ptr<TStoragePool> & pool);

    TStoragePool * primaryStorages();
    TStoragePool * smallStorages();
    TStoragePool * blitStorages();
    TStoragePool * multiBlitStorages();
    TStoragePool * guiThreadStorages();

    void initTexturePool(TexturePoolParams const & p, scoped_ptr<TTexturePool> & pool);

    TTexturePool * primaryTextures();
    TTexturePool * fontTextures();
    TTexturePool * renderTargetTextures();
    TTexturePool * styleCacheTextures();
    TTexturePool * guiThreadTextures();

    shared_ptr<gl::BaseTexture> const & getTexture(string const & fileName);

    Params const & params() const;

    shared_ptr<GlyphInfo> const getGlyphInfo(GlyphKey const & key);
    GlyphMetrics const getGlyphMetrics(GlyphKey const & key);
    GlyphCache * glyphCache(int threadSlot = 0);

    int renderThreadSlot(int threadNum) const;
    int guiThreadSlot() const;
    int cacheThreadSlot() const;

    void addFonts(vector<string> const & fontNames);

    void memoryWarning();
    void enterBackground();
    void enterForeground();

    void updatePoolState();

    void cancel();

    bool useReadPixelsToSynchronize() const;

    shared_ptr<graphics::gl::BaseTexture> createRenderTarget(unsigned w, unsigned h);
    gl::ProgramManager * programManager(int threadSlot);
  };

  Skin * loadSkin(shared_ptr<ResourceManager> const & resourceManager,
                  string const & fileName);
}

