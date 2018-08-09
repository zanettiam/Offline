# Note: the jobs created by this script need at least 8GBytes of memory
generate_fcl --description CeEndpointMix --dsconf MDC2018a --dsowner mu2e --include JobConfig/mixing/CeEndpointMix.fcl \
--run-number 1002 --events-per-job 500 --njobs 5000 --max-engines 30 \
--auxinput=1:physics.filters.ootMixerCRV.fileNames:oot-CRV-cat.txt \
--auxinput=1:physics.filters.neutronMixerCRV.fileNames:neutron-CRV-cat.txt \
--auxinput=1:physics.filters.dioMixerCRV.fileNames:dio-CRV-cat.txt \
--auxinput=1:physics.filters.photonMixerCRV.fileNames:photon-CRV-cat.txt \
--auxinput=1:physics.filters.PSMixerCRV.fileNames:PS-CRV-cut.txt \
--auxinput=1:physics.filters.TSMixerCRV.fileNames:TS-CRV-cat.txt \
--auxinput=1:physics.filters.DSMixerCRV.fileNames:DS-CRV-cut.txt \
--auxinput=1:physics.filters.ootMixerTrkCal.fileNames:oot-TrkCal-cat.txt \
--auxinput=1:physics.filters.neutronMixerTrkCal.fileNames:neutron-TrkCal-cat.txt \
--auxinput=1:physics.filters.dioMixerTrkCal.fileNames:dio-TrkCal-cat.txt \
--auxinput=1:physics.filters.photonMixerTrkCal.fileNames:photon-TrkCal-cat.txt \
--auxinput=10:physics.filters.flashMixerTrkCal.fileNames:DS-flash-TrkCal-cut.txt \
--auxinput=1:physics.filters.protonMixerTrkCal.fileNames:proton-TrkCal.txt \
--auxinput=1:physics.filters.deuteronMixerTrkCal.fileNames:deuteron-TrkCal.txt 
rm -rf CeEndpointMix
mv 000 CeEndpointMix
