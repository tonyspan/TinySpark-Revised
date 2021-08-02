#include <grpcpp/grpcpp.h>

#include "../Core/pch.h"
#include "../Core/SharedHeaders.h"

#include "Worker.h"

Worker::Worker(std::shared_ptr<grpc::Channel> channel) : m_Stub(Utils::UtilsService::NewStub(channel)) {}

Worker::~Worker() {}

void Worker::registerWorker(const std::string &addr)
{
    Utils::RegisterRequest request;
    Utils::RegisterResponse response;
    grpc::ClientContext context;

    request.set_workerhostport(addr);

    grpc::Status status = m_Stub->registerWorker(&context, request, &response);

    if (status.ok())
        std::cout << "Worker (uknown ID yet)" << ": Sent my address" << std::endl;
    else
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;

    m_WorkerID = response.workerid();
}

void Worker::deRegisterWorker(const std::string &addr)
{
    Utils::RegisterRequest request;
    Utils::RegisterResponse response;
    grpc::ClientContext context;

    request.set_workerhostport(addr);

    grpc::Status status = m_Stub->registerWorker(&context, request, &response);

    if (status.ok())
        std::cout << "Worker " << m_WorkerID << ": Removed" << std::endl;
    else
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
}

uint32_t Worker::GetWorkerID() { return m_WorkerID; }

grpc::Status MFRServiceImplWorker::MFRListDW(grpc::ServerContext *context, const MFR::MFRListRequest *request, MFR::MFRResponse *response)
{
    TinySpark::Timer t;
    
    Log("Worker: Received Message from Driver");

    START_TIMER(t)

    if (request->argtype() == TinySpark::Types::NUMERIC)
    {
        std::vector<float> numValues;
        numValues.reserve(request->numlist().size());
        for (auto &it : request->numlist())
            numValues.emplace_back(it);

        auto funcID = Random(1, 100);
        TinySpark::FunctionFactory::GetInstance().AddFunction(TinySpark::FuncComp{  funcID, request->mapform(), request->mapret(),
                                                                                    request->reduceform(), request->reduceret(),
                                                                                    request->filterform(), request->filterret(),
                                                                                    request->hasfilter(), TinySpark::Types::Types::NUMERIC });

        TinySpark::FunctionFactory::GetInstance().BuildFunction();
        TinySpark::FunctionFactory::GetInstance().CompileFunction();

        TinySpark::DLLoader dll;
        std::string currPath = std::filesystem::current_path().make_preferred().string();
        dll.LoadFile(currPath + "/f" + std::to_string(funcID) + ".so");

        std::shared_ptr<TinySpark::MFR<float>> mr(new TinySpark::MFR<float>(numValues));

        // filter
        if (TinySpark::Types::to_bool(request->hasfilter()))
        {
            auto filterFunction = dll.Get<float, float>("filter" + std::to_string(funcID));
            mr->MFR_FilterList<float, float>(filterFunction);

            Log("Worker: Done with Filter Request" );
        }

        // map
        auto mapFunction = dll.Get<float, float>("map" + std::to_string(funcID));
        
        mr->MFR_MapList<float, float>(mapFunction);

        Log("Worker: Done with Map Request");

        auto reduceFunction = dll.Get<float, float, float>("reduce" + std::to_string(funcID));

        // reduce
        auto rr = mr->MFR_ReduceList<float>(reduceFunction, request->initial());

        Log("Worker: Done with Reduce Request");

        response->set_reduceresult(rr);

        Log("Worker: Sent Reply to Driver");

        TIME_ELAPSED(t)

        TinySpark::FunctionFactory::GetInstance().CleanUpFunction();

        return grpc::Status::OK;
    }
    else
    {
        std::vector<std::string> strValues;
        strValues.reserve(request->strlist().size());
        for (auto &it : request->strlist())
            strValues.emplace_back(it);

        auto funcID = Random(1, 100);
        TinySpark::FunctionFactory::GetInstance().AddFunction(TinySpark::FuncComp{  funcID, request->mapform(), request->mapret(),
                                                                                    request->reduceform(), request->reduceret(),
                                                                                    request->filterform(), request->filterret(), 
                                                                                    request->hasfilter(), TinySpark::Types::Types::STRING });

        TinySpark::FunctionFactory::GetInstance().BuildFunction();
        TinySpark::FunctionFactory::GetInstance().CompileFunction();

        TinySpark::DLLoader dll;
        std::string currPath = std::filesystem::current_path().make_preferred().string();
        dll.LoadFile(currPath + "/f" + std::to_string(funcID) + ".so");

        std::shared_ptr<TinySpark::MFR<std::string>> mr(new TinySpark::MFR<std::string>(strValues));

        // filter
        // Not used
        if (TinySpark::Types::to_bool(request->hasfilter()))
        {
            Log("Worker: Done with Filter Request");
        }

        // map
        auto mapFunction = dll.Get<float, std::string>("map" + std::to_string(funcID));
        
        mr->MFR_MapList<float, std::string>(mapFunction);

        Log("Worker: Done with Map Request");

        auto reduceFunction = dll.Get<float, float, float>("reduce" + std::to_string(funcID));

        // reduce
        auto rr = mr->MFR_ReduceList<float>(reduceFunction, request->initial());

        Log("Worker: Done with Reduce Request");

        response->set_reduceresult(rr);

        Log("Worker: Sent Reply to Driver");

        TIME_ELAPSED(t)
        
        TinySpark::FunctionFactory::GetInstance().CleanUpFunction();

        return grpc::Status::OK;
    }

    TIME_ELAPSED(t)

    return grpc::Status(grpc::StatusCode::UNKNOWN, "TinySpark: Something went wrong");

}

grpc::Status MFRServiceImplWorker::MFRFileDW(grpc::ServerContext *context, const MFR::MFRFileRequest *request, MFR::MFRResponse *response)
{
    TinySpark::Timer t;

    Log("Worker: Received message from Driver");
    START_TIMER(t)

    TinySpark::FileHandler fh;
    std::vector<std::string> vecStr;
    std::vector<char> vecChar;

    std::string delims;

    (request->map() == "wc") ? delims = " !\\\"#$%&’\'()*+,-./:;?@[]^_`{|}~=<>\n\t\v\f\r" : delims = "\n";

    fh.OpenFile(request->filename(), false);

    vecChar = fh.ParseFile(fh.GetPathForOpenedFile(), request->chunksize(), request->chunkoffset());

    std::string ss(vecChar.begin(), vecChar.end());

    vecChar.clear();
    vecChar.shrink_to_fit();

    vecStr = fh.TokenizeFile(ss, delims, false);

    ss.clear();
    ss.shrink_to_fit();

    std::shared_ptr<TinySpark::MFR<std::string>> mr(new TinySpark::MFR<std::string>(vecStr));

    mr->MFR_MapFile<std::string, std::string>([](const std::string &el) { return std::make_pair(el, 1); });

    mr->MFR_ReduceFile<std::string, std::string>(request->map());

    if (request->map() == "wc")
    {
        auto mm = mr->MFR_ReturnMap<std::string>();

        Log("Worker: Sending Reply to Driver");

        for (auto &it : mm)
            (*response->mutable_final())[it.first] = it.second;

        fh.CloseFile();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }
    else
    {
        auto rr = mr->MFR_ReturnNonMap<uint32_t>();

        Log("Worker: Sending Reply to Driver");

        response->set_reduceresult(rr);

        fh.CloseFile();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }

    TIME_ELAPSED(t)

    return grpc::Status(grpc::StatusCode::UNKNOWN, "TinySpark: Something went wrong");
}

void Run(const std::string &driverAddr)
{
    const std::string address = "0.0.0.0:" + std::to_string(Random(50052, 50100));
    const unsigned int interval = 10;

    MFRServiceImplWorker service;

    grpc::ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    auto channel = grpc::CreateChannel(driverAddr, grpc::InsecureChannelCredentials());
    Worker worker(channel);

    worker.registerWorker(address);

    Log("Worker", worker.GetWorkerID(), "listening on:", address);

    std::thread th([&]() { server->Wait(); });
    // sleep(interval);
    // server->Shutdown();
    th.join();
}

int main(int argc, char **argv)
{

    std::string driverAddr = "0.0.0.0:50051";

    if (argc > 2)
    {
        Log("./<executable> <driver address to connect>");
        std::exit(EXIT_FAILURE);
    }

    if (argv[1] != NULL)
        driverAddr = argv[1];

    Run(driverAddr);
    
    return 0;
}
