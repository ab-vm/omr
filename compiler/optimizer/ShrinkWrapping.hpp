/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2017
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#ifndef SHRINKWRAP_INCL
#define SHRINKWRAP_INCL

#include <stddef.h>                           // for NULL
#include <stdint.h>                           // for int32_t, int16_t
#include "cs2/hashtab.h"                      // for HashTable
#include "env/TRMemory.hpp"                   // for TR_Memory, etc
#include "il/DataTypes.hpp"                   // for TR_YesNoMaybe
#include "infra/Link.hpp"                     // for TR_Link, TR_LinkHead
#include "optimizer/Optimization.hpp"         // for Optimization
#include "optimizer/OptimizationManager.hpp"  // for OptimizationManager

class TR_BitVector;
class TR_RegisterAnticipatability;
class TR_RegisterAvailability;
namespace TR { class Block; }
namespace TR { class CFG; }
namespace TR { class CFGEdge; }
namespace TR { class Instruction; }
template <class T> class List;

struct PreservedRegisterInfo : public TR_Link<PreservedRegisterInfo>
   {
   int32_t                               _index;
   TR_BitVector                         *_saveBlocks;
   TR_BitVector                         *_restoreBlocks;
   bool                                 _noShrinkWrap;
   };

struct ReturnBlockInfo : public TR_Link<ReturnBlockInfo>
   {
   int32_t _blockNum;
   bool    _restoresRegisters;
   int16_t _numRestoredRegisters;
   };

struct SWEdgeInfo : public TR_Link<SWEdgeInfo>
   {
   TR::CFGEdge * _edge;
   TR::Instruction *_saveSplitLocation;
   TR::Instruction *_restoreSplitLocation;
   TR_BitVector   *_savedRegs;
   TR_BitVector   *_restoredRegs;
   };

class TR_ShrinkWrap : public TR::Optimization
   {
   public:

   TR_ShrinkWrap(TR::OptimizationManager *manager);
   static TR::Optimization *create(TR::OptimizationManager *manager)
      {
      return new (manager->allocator()) TR_ShrinkWrap(manager);
      }

   virtual int32_t perform();
   virtual const char * optDetailString() const throw();

   void prePerformOnBlocks();
   void analyzeInstructions();
   TR::Instruction *findJumpInstructionsInBlock (int32_t blockNum, TR::list<TR::Instruction*> *jmpInstrs);
   void findJumpInstructionsInCodeRegion (TR::Instruction *firstInstr, TR::Instruction *endInstr, CS2::HashTable<TR::Instruction *, bool, TR::Allocator> &setOfJumps);
   void computeSaveRestoreSets(TR_RegisterAnticipatability &registerAnticipatability, TR_RegisterAvailability &registerAvailability);
   void doPlacement(TR_RegisterAnticipatability &registerAnticipatability, TR_RegisterAvailability &registerAvailability);
   PreservedRegisterInfo *findPreservedRegisterInfo(int32_t regIndex);
   TR_YesNoMaybe blockEndsInReturn(int32_t blockNum, bool &hasExceptionSuccs);
   void markInstrsInBlock(int32_t blockNum);
   int32_t updateMapWithRSD(TR::Instruction *instr, int32_t rsd);
   void processInstructionsInSnippet(TR::Instruction *instr, int32_t blockNum);
   TR::Instruction *findReturnInBlock(int32_t blockNum);
   ReturnBlockInfo *findReturnBlockInfo(int32_t blockNum);
   SWEdgeInfo *findEdgeInfo(TR::CFGEdge *e);
   void composeSavesRestores();
   bool findMultiples(TR_BitVector *regs, TR::Instruction *startLocation, bool doSaves, bool forwardWalk = true);

   TR_BitVector **_saveInfo;
   TR_BitVector **_restoreInfo;

   struct SWBlockInfo
      {
      TR_ALLOC(TR_Memory::ShrinkWrapping)

      SWBlockInfo() : _block(NULL),
                      _startInstr(NULL),
                      _endInstr(NULL),
                      _savedRegs(NULL),
                      _restoredRegs(NULL),
                      _restoreLocation(NULL) {}
      TR::Block * _block;
      TR::Instruction *_startInstr;
      TR::Instruction *_endInstr;
      TR_BitVector *_savedRegs;
      TR_BitVector *_restoredRegs;
      TR::Instruction *_restoreLocation;
      };

   private:

   TR_BitVector                        **_registerUsageInfo;
   int32_t                               _numberOfNodes;
   SWBlockInfo                          *_swBlockInfo;
   TR::Block                            **_cfgBlocks;
   int32_t                              *_mapRegsToStack;
   int32_t                               _numPreservedRegs;
   TR_LinkHead<PreservedRegisterInfo>    _preservedRegs;
   TR::CFG                               *_cfg;
   bool                                  _traceSW;
   TR_BitVector                         *_preservedRegsInMethod;
   TR_BitVector                         *_preservedRegsInLinkage;
   TR_LinkHead<ReturnBlockInfo>         _returnBlocks;
   TR_LinkHead<SWEdgeInfo>              _swEdgeInfo;
   bool                                 _linkageUsesPushes;
   };


#endif
