#include <grpcpp/grpcpp.h>

#include <omp.h>

#include "../Core/pch.h"
#include "../Core/SharedHeaders.h"

#include "Driver.h"

std::vector<std::shared_ptr<TinySpark::ServerRegistry>> g_WorkerList;

grpc::Status DiscoverSericeImplDriver::registerWorker(grpc::ServerContext *context, const Utils::RegisterRequest *request, Utils::RegisterResponse *response)
{

    std::shared_ptr<TinySpark::ServerRegistry> ep(new TinySpark::ServerRegistry());

    ep->SetHostAndPort(request->workerhostport());
    ep->SetLoad(false);
    ep->SetAlive(true);
    
    g_WorkerList.emplace_back(ep);
    g_WorkerList.erase(std::unique(g_WorkerList.begin(), g_WorkerList.end()), g_WorkerList.end());

    Log("Driver: Worker's", ep->GetID(), "IP, port: ", request->workerhostport());

    response->set_workerid(ep->GetID());

    return grpc::Status::OK;
}

// TODO: Probably causes duplicate instances of workers in g_WorkerList
grpc::Status DiscoverSericeImplDriver::deRegisterWorker(grpc::ServerContext *context, const Utils::RegisterRequest *request, Utils::EmptyResponse *response)
{

    auto it = std::find_if(g_WorkerList.begin(), g_WorkerList.end(), [&](auto &elem) { return elem->GetHostAndPort() == request->workerhostport(); });
    
    if (it != g_WorkerList.end())
        g_WorkerList.erase(it);

    return grpc::Status::OK;
}

grpc::Status MFRServiceImplDriver::MFRListCD(grpc::ServerContext *context, const MFR::MFRListRequest *request, MFR::MFRResponse *response)
{
    TinySpark::Timer t;

    Log("Driver: Received Request from Client");
    
    START_TIMER(t)

    auto luckyOne = g_WorkerList.at(Random(0, g_WorkerList.size() - 1));
    auto channel = grpc::CreateChannel(luckyOne->GetHostAndPort(), grpc::InsecureChannelCredentials());
    Driver driver;

    driver.SetChannel(channel, "MFR");

    if (request->argtype() == TinySpark::Types::NUMERIC)
    {
        std::vector<float>  NumValues;
        for (auto &it : request->numlist())
            NumValues.emplace_back(it);

        auto rr = driver.MFRListDW(NumValues, DataToSent{   request->mapform(), request->mapret(),
                                                            request->reduceform(), request->reduceret(),
                                                            request->filterform(), request->filterret(),
                                                            request->hasfilter(), request->argtype(), request->initial() });

        Log("Driver: Sent Reply to Client");
        
        response->set_reduceresult(rr);

        NumValues.clear();
        NumValues.shrink_to_fit();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }
    else
    {
        std::vector<std::string>  strValues;
        for (auto &it : request->strlist())
            strValues.emplace_back(it);

        auto rr = driver.MFRListDW(strValues, DataToSent{   request->mapform(), request->mapret(),
                                                            request->reduceform(), request->reduceret(),
                                                            std::string(""), std::string(""),
                                                            request->hasfilter(), request->argtype(), request->initial() });

        Log("Driver: Sent Reply to Client");
        
        response->set_reduceresult(rr);

        strValues.clear();
        strValues.shrink_to_fit();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }

    TIME_ELAPSED(t)

    return grpc::Status(grpc::StatusCode::UNKNOWN, "TinySpark: Something went wrong");
}

grpc::Status MFRServiceImplDriver::MFRFileCD(grpc::ServerContext *context, const MFR::MFRFileRequest *request, MFR::MFRResponse *response)
{
    TinySpark::Timer t;

    Log("Driver: Received Map, Reduce Request from Client");
    
    START_TIMER(t)

    TinySpark::FileHandler fh;

    std::shared_ptr<grpc::Channel> channel;
    Driver driver;

    typedef std::unordered_map<std::string, uint32_t> map;
    map mapCombiner;
    map mapRecved;

    uint32_t numCombiner = 0;
    uint32_t numRecved = 0;

    fh.OpenFile(request->filename());

    // OpenMP doesn't like objects
    auto chunks = fh.GetChunks();
    auto filename = fh.GetFileName();
    auto chunkSize = fh.GetChunkSizeBytes();
    auto lastChunkSize = fh.GetLastChunkSizeBytes();
    auto workerListSize = g_WorkerList.size();
    auto mapAction = request->map();
    auto thisChunkSize = 0;
    auto who = 0;

    if (request->map() == "wc")
    {
        #pragma omp parallel for shared(chunks, filename, lastChunkSize, chunkSize, workerListSize, mapAction, mapCombiner) private(thisChunkSize, who, channel)
        for (auto i = 0; i < chunks; i++)
        {
            thisChunkSize = (i == chunks - 1) ? lastChunkSize : chunkSize;
            who = i % workerListSize;

            channel = grpc::CreateChannel(g_WorkerList[who]->GetHostAndPort(), grpc::InsecureChannelCredentials());
            driver.SetChannel(channel, "MFR");
            mapRecved = driver.MFRFileMapDW(filename, mapAction, thisChunkSize, i * chunkSize);

            // combiner
            #pragma omp critical
            for (auto &it : mapRecved)
                mapCombiner[it.first] = mapCombiner[it.first] + it.second;
        }

        Log("Driver: Sending Reply to Client");
        for (auto &it : mapCombiner)
            (*response->mutable_final())[it.first] = it.second;

        mapCombiner.clear();
        mapRecved.clear();

        fh.CloseFile();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }
    else
    {
        #pragma omp parallel for shared(chunks, filename, lastChunkSize, chunkSize, workerListSize, mapAction) reduction(+: numCombiner) private(thisChunkSize, who, channel)
        for (auto i = 0; i < chunks; i++)
        {
            thisChunkSize = (i == chunks - 1) ? lastChunkSize : chunkSize;
            who = i % workerListSize;

            channel = grpc::CreateChannel(g_WorkerList[who]->GetHostAndPort(), grpc::InsecureChannelCredentials());
            driver.SetChannel(channel, "MFR");
            numRecved = driver.MFRFileNonMapDW(filename, mapAction, thisChunkSize, i * chunkSize);

            // combiner
            numCombiner += numRecved;
        }

        Log("Driver: Sending Reply to Client");
        response->set_reduceresult(numCombiner);

        mapCombiner.clear();
        mapRecved.clear();

        fh.CloseFile();

        TIME_ELAPSED(t)

        return grpc::Status::OK;
    }

    TIME_ELAPSED(t)

    return grpc::Status(grpc::StatusCode::UNKNOWN, "TinySpark: Something went wrong");
}

Driver::Driver() {}

Driver::~Driver() {}

void Driver::SetChannel(std::shared_ptr<grpc::Channel> channel, const std::string& service)
{
    if(service == "MFR") 
    {
        m_MFRStub = MFR::MFRService::NewStub(channel);
    }
    else
    {
        m_UtilsStub = Utils::UtilsService::NewStub(channel);
    }
}

float Driver::MFRListDW(const std::vector<float> &values, const DataToSent &data)
{
    MFR::MFRListRequest request;
    MFR::MFRResponse response;
    grpc::ClientContext context;

    for (auto &it : values)
        request.add_numlist(it);
    
    request.set_mapform(data.MapForm);
    request.set_mapret(data.MapRet);
    request.set_filterform(data.FilterForm);
    request.set_filterret(data.FilterRet);
    request.set_reduceform(data.ReduceForm);
    request.set_reduceret(data.ReduceRet);
    request.set_hasfilter(data.HasFilter);
    request.set_argtype(data.ArgType);
    request.set_initial(data.initial);

    grpc::Status status = m_MFRStub->MFRListDW(&context, request, &response);

    Log("Driver: Waiting for worker to respond");
    
    if (status.ok())
        Log("Driver: Worker got my Message");
    else
        Log(status.error_code(), ":", status.error_message());

    Log("Driver: Worker responded");

    return response.reduceresult();
}

float Driver::MFRListDW(const std::vector<std::string> &values, const DataToSent &data)
{
    MFR::MFRListRequest request;
    MFR::MFRResponse response;
    grpc::ClientContext context;

    for (auto &it : values)
        request.add_strlist(it);
    
    request.set_mapform(data.MapForm);
    request.set_mapret(data.MapRet);
    request.set_reduceform(data.ReduceForm);
    request.set_reduceret(data.ReduceRet);
    request.set_hasfilter(data.HasFilter);
    request.set_argtype(data.ArgType);
    request.set_initial(data.initial);

    grpc::Status status = m_MFRStub->MFRListDW(&context, request, &response);

    Log("Driver: Waiting for worker to respond");
    
    if (status.ok())
        Log("Driver: Worker got my Message");
    else
        Log(status.error_code(), ":", status.error_message());

    Log("Driver: Worker responded");

    return response.reduceresult();
}

std::unordered_map<std::string, uint32_t> Driver::MFRFileMapDW(const std::string &filename, const std::string &mapAction, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset)
{
    MFR::MFRFileRequest request;
    MFR::MFRResponse response;
    grpc::ClientContext context;

    request.set_map(mapAction);
    request.set_filename(filename);
    request.set_chunksize(chunkSize);
    request.set_chunkoffset(chunkOffset);

    grpc::Status status = m_MFRStub->MFRFileDW(&context, request, &response);

    if (status.ok())
        Log("Driver: Worker got my message");
    else
        Log(status.error_code(), ":", status.error_message());

    Log("Driver: Waiting for worker to respond");

    Log("Driver: Worker responded");

    typedef std::unordered_map<std::string, uint32_t> map;
    map mapReturn;

    for (auto &it : response.final())
        mapReturn[it.first] = it.second;

    return mapReturn;
}

uint32_t Driver::MFRFileNonMapDW(const std::string &filename, const std::string &mapAction, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset)
{
    MFR::MFRFileRequest request;
    MFR::MFRResponse response;
    grpc::ClientContext context;

    request.set_map(mapAction);
    request.set_filename(filename);
    request.set_chunksize(chunkSize);
    request.set_chunkoffset(chunkOffset);

    grpc::Status status = m_MFRStub->MFRFileDW(&context, request, &response);

    if (status.ok())
        Log("Driver: Worker got my message");
    else
        Log(status.error_code(), ":",status.error_message());

    Log("Driver: Waiting for worker to respond");

    Log("Driver: Worker responded");

    return response.reduceresult();
}

void Run(const std::string& address)
{
    // const std::string address = "0.0.0.0:50051";
    const unsigned int interval = 10;

    DiscoverSericeImplDriver service;
    MFRServiceImplDriver service2;
    
    grpc::ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    builder.RegisterService(&service2);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    Log("Driver listening on ", address);

    std::thread th([&]() { server->Wait(); });
    
    // Maybe doesn't do what I want
    sleep(interval);
    
    th.join();
}

int main(int argc, char **argv)
{
    std::string address = "0.0.0.0:50051";
    
    if(argc > 2)
        std::exit(EXIT_FAILURE);

    if(argv[1] != NULL)
        address = argv[1];
    
    Run(address);
    return 0;
}