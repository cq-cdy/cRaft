// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: raft.proto
#ifndef GRPC_raft_2eproto__INCLUDED
#define GRPC_raft_2eproto__INCLUDED

#include "raft.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

// service
class RaftRPC final {
 public:
  static constexpr char const* service_full_name() {
    return "RaftRPC";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::RequestVoteReply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>> AsyncrequestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>>(AsyncrequestVoteRPCRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>> PrepareAsyncrequestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>>(PrepareAsyncrequestVoteRPCRaw(context, request, cq));
    }
    virtual ::grpc::Status appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::AppendEntriesReply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>> AsyncappendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>>(AsyncappendEntriesRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>> PrepareAsyncappendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>>(PrepareAsyncappendEntriesRaw(context, request, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response, std::function<void(::grpc::Status)>) = 0;
      virtual void requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      virtual void appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response, std::function<void(::grpc::Status)>) = 0;
      virtual void appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response, ::grpc::ClientUnaryReactor* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>* AsyncrequestVoteRPCRaw(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::RequestVoteReply>* PrepareAsyncrequestVoteRPCRaw(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>* AsyncappendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::AppendEntriesReply>* PrepareAsyncappendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::RequestVoteReply* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>> AsyncrequestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>>(AsyncrequestVoteRPCRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>> PrepareAsyncrequestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>>(PrepareAsyncrequestVoteRPCRaw(context, request, cq));
    }
    ::grpc::Status appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::AppendEntriesReply* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>> AsyncappendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>>(AsyncappendEntriesRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>> PrepareAsyncappendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>>(PrepareAsyncappendEntriesRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response, std::function<void(::grpc::Status)>) override;
      void requestVoteRPC(::grpc::ClientContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response, ::grpc::ClientUnaryReactor* reactor) override;
      void appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response, std::function<void(::grpc::Status)>) override;
      void appendEntries(::grpc::ClientContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>* AsyncrequestVoteRPCRaw(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::RequestVoteReply>* PrepareAsyncrequestVoteRPCRaw(::grpc::ClientContext* context, const ::RequestVoteArgs& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>* AsyncappendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::AppendEntriesReply>* PrepareAsyncappendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesArgs& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_requestVoteRPC_;
    const ::grpc::internal::RpcMethod rpcmethod_appendEntries_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status requestVoteRPC(::grpc::ServerContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response);
    virtual ::grpc::Status appendEntries(::grpc::ServerContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestrequestVoteRPC(::grpc::ServerContext* context, ::RequestVoteArgs* request, ::grpc::ServerAsyncResponseWriter< ::RequestVoteReply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_appendEntries() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestappendEntries(::grpc::ServerContext* context, ::AppendEntriesArgs* request, ::grpc::ServerAsyncResponseWriter< ::AppendEntriesReply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_requestVoteRPC<WithAsyncMethod_appendEntries<Service > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::RequestVoteArgs, ::RequestVoteReply>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::RequestVoteArgs* request, ::RequestVoteReply* response) { return this->requestVoteRPC(context, request, response); }));}
    void SetMessageAllocatorFor_requestVoteRPC(
        ::grpc::MessageAllocator< ::RequestVoteArgs, ::RequestVoteReply>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::RequestVoteArgs, ::RequestVoteReply>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* requestVoteRPC(
      ::grpc::CallbackServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_appendEntries() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::AppendEntriesArgs, ::AppendEntriesReply>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::AppendEntriesArgs* request, ::AppendEntriesReply* response) { return this->appendEntries(context, request, response); }));}
    void SetMessageAllocatorFor_appendEntries(
        ::grpc::MessageAllocator< ::AppendEntriesArgs, ::AppendEntriesReply>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::AppendEntriesArgs, ::AppendEntriesReply>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* appendEntries(
      ::grpc::CallbackServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_requestVoteRPC<WithCallbackMethod_appendEntries<Service > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_appendEntries() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestrequestVoteRPC(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_appendEntries() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestappendEntries(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->requestVoteRPC(context, request, response); }));
    }
    ~WithRawCallbackMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* requestVoteRPC(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_appendEntries() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->appendEntries(context, request, response); }));
    }
    ~WithRawCallbackMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* appendEntries(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_requestVoteRPC : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_requestVoteRPC() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::RequestVoteArgs, ::RequestVoteReply>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::RequestVoteArgs, ::RequestVoteReply>* streamer) {
                       return this->StreamedrequestVoteRPC(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_requestVoteRPC() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status requestVoteRPC(::grpc::ServerContext* /*context*/, const ::RequestVoteArgs* /*request*/, ::RequestVoteReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedrequestVoteRPC(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::RequestVoteArgs,::RequestVoteReply>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_appendEntries : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_appendEntries() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::AppendEntriesArgs, ::AppendEntriesReply>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::AppendEntriesArgs, ::AppendEntriesReply>* streamer) {
                       return this->StreamedappendEntries(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_appendEntries() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status appendEntries(::grpc::ServerContext* /*context*/, const ::AppendEntriesArgs* /*request*/, ::AppendEntriesReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedappendEntries(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::AppendEntriesArgs,::AppendEntriesReply>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_requestVoteRPC<WithStreamedUnaryMethod_appendEntries<Service > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_requestVoteRPC<WithStreamedUnaryMethod_appendEntries<Service > > StreamedService;
};


#endif  // GRPC_raft_2eproto__INCLUDED