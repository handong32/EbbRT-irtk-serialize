#ifndef _EBBRT_COEFF_INIT_H_
#define _EBBRT_COEFF_INIT_H_

#define NOMINMAX
#define _USE_MATH_DEFINES

#include <irtkReconstructionGPU.h>
#include <irtkResampling.h>
#include <irtkRegistration.h>
#include <irtkImageRigidRegistration.h>
#include <irtkImageRigidRegistrationWithPadding.h>
#include <irtkImageFunction.h>
#include <irtkTransformation.h>
#include <math.h>
#include <stdlib.h>
#include <utils.h>
#include <string>

#include <ebbrt/GlobalIdMap.h>
#include <ebbrt/StaticIds.h>
#include <ebbrt/EbbRef.h>
#include <ebbrt/SharedEbb.h>
#include <ebbrt/Message.h>
#include <ebbrt/UniqueIOBuf.h>
//#include "../../src/StaticEbbIds.h"
#include "Printer.h"

#include <iostream>

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace ebbrt;

class EbbRTCoeffInit;
typedef EbbRef<EbbRTCoeffInit> EbbRTCoeffInitEbbRef;

class EbbRTCoeffInit : public SharedEbb<EbbRTCoeffInit>,
                       public Messagable<EbbRTCoeffInit> {

  irtkReconstruction* reconstructor;
  EbbId ebbid;
  std::string bmnid;
  irtkMatrix mym;
  std::vector<ebbrt::Messenger::NetworkId> nids;
  int numNodes;
  int recv_counter;

  irtkRealImage _slice;
  irtkRigidTransformation _transformations;
  SLICECOEFFS slicecoeffs;
  bool slice_inside;

  // this is used to save and load context
  ebbrt::EventManager::EventContext* emec{nullptr};

  // a void promise where we want to start some computation after some other
  // function has finished
  ebbrt::Promise<void> mypromise;
  ebbrt::Promise<void> nodesinit;

 public:
  EbbRTCoeffInit(irtkReconstruction* _reconstructor, EbbId _ebbid, int _num)
      : Messagable<EbbRTCoeffInit>(_ebbid), reconstructor(_reconstructor) {
    ebbid = _ebbid;
    bmnid = "";
    numNodes = _num;
    recv_counter = 0;
    nids.clear();
  }

  // Create function that returns a future ebbref
  static ebbrt::Future<EbbRTCoeffInitEbbRef>
  Create(irtkReconstruction* _reconstructor, int num) {
    // calls the constructor in this class and returns a EbbRef
    auto id = ebb_allocator->Allocate();
    auto ebbref = SharedEbb<EbbRTCoeffInit>::Create(
        new EbbRTCoeffInit(_reconstructor, id, num), id);

    // returns a future for the EbbRef in AppMain.cc
    return ebbrt::global_id_map
        ->Set(id, ebbrt::messenger->LocalNetworkId().ToBytes())
        // the Future<void> f is returned by the ebbrt::global_id_map
        .Then([ebbref](ebbrt::Future<void> f) {
          // f.Get() ensures the gobal_id_map has completed
          f.Get();
          return ebbref;
        });
  }

  // this uses the void promise to return a future that will
  // get invoked when mypromose gets set with some value
  ebbrt::Future<void> waitReceive() { return std::move(mypromise.GetFuture()); }
  // FIXME
  ebbrt::Future<void> waitNodes() { return std::move(nodesinit.GetFuture()); }

  void addNid(ebbrt::Messenger::NetworkId nid) {
      nids.push_back(nid);
      if ((int)nids.size() == numNodes) {
	  nodesinit.SetValue();
      }
  }

  void Print(ebbrt::Messenger::NetworkId nid, const char* str);
  
  void ReceiveMessage(ebbrt::Messenger::NetworkId nid,
                      std::unique_ptr<ebbrt::IOBuf>&& buffer);

  EbbId getEbbId() { return ebbid; }

  void initMatrix(irtkMatrix& m, double val, int row, int col) {
    for (int i = 0; i < row; i++) {
      for (int j = 0; j < col; j++) {
        m.Put(i, j, val);
      }
    }
  }

  void runJob(int);
  void runJob2();
  void runJob3();
  void coeffinit();
  void coeffinit2();
  void coeffinitParallel();
  void coeffinitParallel2();

  void destroy() { delete this; }
};

#endif
