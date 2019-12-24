#include <cstdio>
#include <memory>
#include <vector>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenWeightInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenLumiInfoHeader.h"

#include "GeneratorInterface/Core/interface/GenWeightHelper.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

class GenWeightProductProducer : public edm::one::EDProducer<edm::BeginLuminosityBlockProducer> {
public:
  explicit GenWeightProductProducer(const edm::ParameterSet& iConfig);
  ~GenWeightProductProducer() override;
  
private:
  std::vector<std::string> weightNames_;
  gen::GenWeightHelper weightHelper_;
  edm::EDGetTokenT<GenLumiInfoHeader> genLumiInfoToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genEventToken_;

  void produce(edm::Event&, const edm::EventSetup&) override;
  void beginLuminosityBlockProduce(edm::LuminosityBlock& lb, edm::EventSetup const& c) override;
};

//
// constructors and destructor
//
GenWeightProductProducer::GenWeightProductProducer(const edm::ParameterSet& iConfig) :
    genLumiInfoToken_(consumes<GenLumiInfoHeader, edm::InLumi>(edm::InputTag("generator"))),
        //iConfig.getUntrackedParameter<edm::InputTag>("lheSource", edm::InputTag("externalLHEProducer")))),
    genEventToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator")))
        //iConfig.getUntrackedParameter<edm::InputTag>("lheSource", edm::InputTag("externalLHEProducer"))))
{
  produces<GenWeightProduct>();
  produces<GenWeightInfoProduct, edm::Transition::BeginLuminosityBlock>();
}


GenWeightProductProducer::~GenWeightProductProducer()
{
}


// ------------ method called to produce the data  ------------
void
GenWeightProductProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<GenEventInfoProduct> genEventInfo;
  iEvent.getByToken(genEventToken_, genEventInfo);
  // Read weights from LHEEventProduct
  auto weightProduct = weightHelper_.weightProduct(genEventInfo->weights());
  iEvent.put(std::move(weightProduct));
}

//void 
//GenWeightProductProducer::endLuminosityBlockProduce(edm::LuminosityBlock& iLumi, edm::EventSetup const& iSetup) {}

void
GenWeightProductProducer::beginLuminosityBlockProduce(edm::LuminosityBlock& iLumi, edm::EventSetup const& iSetup) {
    if (weightNames_.size() == 0) {
        edm::Handle<GenLumiInfoHeader> genLumiInfoHandle;
        iLumi.getByToken(genLumiInfoToken_, genLumiInfoHandle);

        weightNames_ = genLumiInfoHandle->weightNames();
	    weightHelper_.parseWeightGroupsFromNames(weightNames_);
    }
    auto weightInfoProduct = std::make_unique<GenWeightInfoProduct>();
    for (auto& weightGroup : weightHelper_.weightGroups()) {
        weightInfoProduct->addWeightGroupInfo(weightGroup.clone());
    }
    iLumi.put(std::move(weightInfoProduct));
}

DEFINE_FWK_MODULE(GenWeightProductProducer);


