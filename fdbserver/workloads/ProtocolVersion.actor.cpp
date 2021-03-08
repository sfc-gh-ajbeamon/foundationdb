/*
 * ProtocolVersion.actor.cpp
 *
 * This source file is part of the FoundationDB open source project
 *
 * Copyright 2013-2019 Apple Inc. and the FoundationDB project authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fdbserver/workloads/workloads.actor.h"

struct ProtocolVersionWorkload : TestWorkload {
    ProtocolVersionWorkload(WorkloadContext const& wcx)
	: TestWorkload(wcx) {
        
    }

	std::string description() const override {
		return "ProtocolVersionWorkload";
	}

	Future<Void> start(Database const& cx) override {
       return _start(this, cx);
	}

    ACTOR Future<Void> _start(ProtocolVersionWorkload* self, Database cx) {
        state ISimulator::ProcessInfo* currProcess = g_pSimulator->getCurrentProcess();
        state std::vector<ISimulator::ProcessInfo*> allProcesses = g_pSimulator->getAllProcesses();
        state std::vector<ISimulator::ProcessInfo*>::iterator diffVersionProcess = find_if(allProcesses.begin(), allProcesses.end(), [](const ISimulator::ProcessInfo* p){
            return p->protocolVersion != currentProtocolVersion;
        });
        
        ASSERT(diffVersionProcess != allProcesses.end());

        wait(g_pSimulator->onProcess(*diffVersionProcess));
        uint64_t version = wait(getCoordinatorProtocols(cx->getConnectionFile(), Optional<ProtocolVersion>()));
        ASSERT(version != g_network->protocolVersion().version());

        // switch back to protocol-compatible process for consistency check
        wait(g_pSimulator->onProcess(currProcess));

        return Void();
	}

    Future<bool> check(Database const& cx) override {
		return true;
	}

	void getMetrics(vector<PerfMetric>& m) override {
	}
};

WorkloadFactory<ProtocolVersionWorkload> ProtocolVersionWorkloadFactory("ProtocolVersion");
 