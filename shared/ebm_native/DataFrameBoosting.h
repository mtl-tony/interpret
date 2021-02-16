// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef DATA_FRAME_BOOSTING_H
#define DATA_FRAME_BOOSTING_H

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t

#include "ebm_native.h" // FloatEbmType
#include "EbmInternal.h" // INLINE_ALWAYS
#include "Logging.h" // EBM_ASSERT & LOG
#include "FeatureGroup.h"

class DataFrameBoosting final {
   FloatEbmType * m_aGradientsAndHessians;
   FloatEbmType * m_aPredictorScores;
   StorageDataType * m_aTargetData;
   StorageDataType * * m_aaInputData;
   size_t m_cSamples;
   size_t m_cFeatureGroups;

public:

   DataFrameBoosting() = default; // preserve our POD status
   ~DataFrameBoosting() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   INLINE_ALWAYS void InitializeZero() {
      m_aGradientsAndHessians = nullptr;
      m_aPredictorScores = nullptr;
      m_aTargetData = nullptr;
      m_aaInputData = nullptr;
      m_cSamples = 0;
      m_cFeatureGroups = 0;
   }

   void Destruct();

   bool Initialize(
      const bool bAllocateGradients, 
      const bool bAllocateHessians,
      const bool bAllocatePredictorScores,
      const bool bAllocateTargetData, 
      const size_t cFeatureGroups, 
      const FeatureGroup * const * const apFeatureGroup, 
      const size_t cSamples, 
      const IntEbmType * const aInputDataFrom, 
      const void * const aTargets, 
      const FloatEbmType * const aPredictorScoresFrom, 
      const ptrdiff_t runtimeLearningTypeOrCountTargetClasses
   );

   INLINE_ALWAYS FloatEbmType * GetGradientsAndHessiansPointer() {
      EBM_ASSERT(nullptr != m_aGradientsAndHessians);
      return m_aGradientsAndHessians;
   }
   INLINE_ALWAYS const FloatEbmType * GetGradientsAndHessiansPointer() const {
      EBM_ASSERT(nullptr != m_aGradientsAndHessians);
      return m_aGradientsAndHessians;
   }
   INLINE_ALWAYS FloatEbmType * GetPredictorScores() {
      EBM_ASSERT(nullptr != m_aPredictorScores);
      return m_aPredictorScores;
   }
   INLINE_ALWAYS const StorageDataType * GetTargetDataPointer() const {
      EBM_ASSERT(nullptr != m_aTargetData);
      return m_aTargetData;
   }
   // TODO: we can change this to take the GetIndexInputData() value directly, which we get from a loop index
   INLINE_ALWAYS const StorageDataType * GetInputDataPointer(const FeatureGroup * const pFeatureGroup) const {
      EBM_ASSERT(nullptr != pFeatureGroup);
      EBM_ASSERT(pFeatureGroup->GetIndexInputData() < m_cFeatureGroups);
      EBM_ASSERT(nullptr != m_aaInputData);
      return m_aaInputData[pFeatureGroup->GetIndexInputData()];
   }
   INLINE_ALWAYS size_t GetCountSamples() const {
      return m_cSamples;
   }
   INLINE_ALWAYS size_t GetCountFeatureGroups() const {
      return m_cFeatureGroups;
   }
};
static_assert(std::is_standard_layout<DataFrameBoosting>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<DataFrameBoosting>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");
static_assert(std::is_pod<DataFrameBoosting>::value,
   "We use a lot of C constructs, so disallow non-POD types in general");

#endif // DATA_FRAME_BOOSTING_H
