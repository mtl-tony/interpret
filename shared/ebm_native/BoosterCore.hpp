// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <ebm@koch.ninja>

#ifndef BOOSTER_CORE_HPP
#define BOOSTER_CORE_HPP

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t
#include <limits> // numeric_limits
#include <atomic>

#include "ebm_native.h"
#include "logging.h"
#include "zones.h"

#include "ebm_internal.hpp"

#include "Tensor.hpp"
// feature includes
#include "Feature.hpp"
// Term.hpp depends on Feature.h
#include "Term.hpp"
// dataset depends on features
#include "DataSetBoosting.hpp"
// samples is somewhat independent from datasets, but relies on an indirect coupling with them
#include "InnerBag.hpp"

namespace DEFINED_ZONE_NAME {
#ifndef DEFINED_ZONE_NAME
#error DEFINED_ZONE_NAME must be defined
#endif // DEFINED_ZONE_NAME

class BoosterShell;

class BoosterCore final {

   // std::atomic_size_t used to be standard layout and trivial, but the C++ standard comitee judged that an error
   // and revoked the trivial nature of the class.  So, this means our BoosterCore class needs to have a constructor 
   // and destructor
   // https://stackoverflow.com/questions/48794325/why-stdatomic-is-not-trivial-type-in-only-visual-c
   // https://stackoverflow.com/questions/41308372/stdatomic-for-built-in-types-non-lock-free-vs-trivial-destructor
   std::atomic_size_t m_REFERENCE_COUNT;

   ptrdiff_t m_cClasses;

   size_t m_cFeatures;
   Feature * m_aFeatures;

   size_t m_cTerms;
   Term ** m_apTerms;

   size_t m_cInnerBags;
   InnerBag ** m_apInnerBags;
   FloatBig m_validationWeightTotal;
   FloatFast * m_aValidationWeights;

   Tensor ** m_apCurrentTermTensors;
   Tensor ** m_apBestTermTensors;

   double m_bestModelMetric;

   size_t m_cBytesFastBins;
   size_t m_cBytesBigBins;

   size_t m_cBytesSplitPositions;
   size_t m_cBytesTreeNodes;

   DataSetBoosting m_trainingSet;
   DataSetBoosting m_validationSet;

   static void DeleteTensors(const size_t cTerms, Tensor ** const apTensors);

   static ErrorEbm InitializeTensors(
      const size_t cTerms,
      const Term * const * const apTerms,
      const size_t cScores,
      Tensor *** papTensorsOut
   );

   INLINE_ALWAYS ~BoosterCore() {
      // this only gets called after our reference count has been decremented to zero

      m_trainingSet.Destruct();
      m_validationSet.Destruct();

      InnerBag::FreeInnerBags(m_cInnerBags, m_apInnerBags);
      free(m_aValidationWeights);

      Term::FreeTerms(m_cTerms, m_apTerms);

      free(m_aFeatures);

      DeleteTensors(m_cTerms, m_apCurrentTermTensors);
      DeleteTensors(m_cTerms, m_apBestTermTensors);
   };

   WARNING_PUSH
   ATTRIBUTE_WARNING_DISABLE_UNINITIALIZED_MEMBER
   INLINE_ALWAYS BoosterCore() noexcept :
      m_REFERENCE_COUNT(1), // we're not visible on any other thread yet, so no synchronization required
      m_cClasses(0),
      m_cFeatures(0),
      m_aFeatures(nullptr),
      m_cTerms(0),
      m_apTerms(nullptr),
      m_cInnerBags(0),
      m_apInnerBags(nullptr),
      m_validationWeightTotal(0),
      m_aValidationWeights(nullptr),
      m_apCurrentTermTensors(nullptr),
      m_apBestTermTensors(nullptr),
      m_bestModelMetric(0),
      m_cBytesFastBins(0),
      m_cBytesBigBins(0),
      m_cBytesSplitPositions(0),
      m_cBytesTreeNodes(0)
   {
      m_trainingSet.InitializeUnfailing();
      m_validationSet.InitializeUnfailing();
   }
   WARNING_POP

public:

   INLINE_ALWAYS void AddReferenceCount() {
      // incrementing reference counts can be relaxed memory order since we're guaranteed to be above 1, 
      // so no result will change our behavior below
      // https://www.boost.org/doc/libs/1_59_0/doc/html/atomic/usage_examples.html
      m_REFERENCE_COUNT.fetch_add(1, std::memory_order_relaxed);
   };

   INLINE_ALWAYS ptrdiff_t GetCountClasses() const {
      return m_cClasses;
   }

   INLINE_ALWAYS size_t GetCountBytesFastBins() const {
      return m_cBytesFastBins;
   }

   INLINE_ALWAYS size_t GetCountBytesBigBins() const {
      return m_cBytesBigBins;
   }

   INLINE_ALWAYS size_t GetCountBytesSplitPositions() const {
      return m_cBytesSplitPositions;
   }

   INLINE_ALWAYS size_t GetCountBytesTreeNodes() const {
      return m_cBytesTreeNodes;
   }

   INLINE_ALWAYS size_t GetCountTerms() const {
      return m_cTerms;
   }

   INLINE_ALWAYS Term * const * GetTerms() const {
      return m_apTerms;
   }

   INLINE_ALWAYS DataSetBoosting * GetTrainingSet() {
      return &m_trainingSet;
   }

   INLINE_ALWAYS DataSetBoosting * GetValidationSet() {
      return &m_validationSet;
   }

   INLINE_ALWAYS size_t GetCountInnerBags() const {
      return m_cInnerBags;
   }

   INLINE_ALWAYS const InnerBag * const * GetInnerBags() const {
      return m_apInnerBags;
   }

   INLINE_ALWAYS FloatBig GetValidationWeightTotal() const {
      return m_validationWeightTotal;
   }

   INLINE_ALWAYS const FloatFast * GetValidationWeights() const {
      return m_aValidationWeights;
   }

   INLINE_ALWAYS Tensor * const * GetCurrentModel() const {
      return m_apCurrentTermTensors;
   }

   INLINE_ALWAYS Tensor * const * GetBestModel() const {
      return m_apBestTermTensors;
   }

   INLINE_ALWAYS double GetBestModelMetric() const {
      return m_bestModelMetric;
   }

   INLINE_ALWAYS void SetBestModelMetric(const double bestModelMetric) {
      m_bestModelMetric = bestModelMetric;
   }

   static void Free(BoosterCore * const pBoosterCore);

   static ErrorEbm Create(
      RandomDeterministic * const pRng,
      BoosterShell * const pBoosterShell,
      const size_t cTerms,
      const size_t cInnerBags,
      const double * const experimentalParams,
      const IntEbm * const acTermDimensions,
      const IntEbm * const aiTermFeatures,
      const unsigned char * const pDataSetShared,
      const BagEbm * const aBag,
      const double * const aInitScores
   );
};

} // DEFINED_ZONE_NAME

#endif // BOOSTER_CORE_HPP
