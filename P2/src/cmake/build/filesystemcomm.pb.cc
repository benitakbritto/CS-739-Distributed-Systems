// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: filesystemcomm.proto

#include "filesystemcomm.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
namespace filesystemcomm {
constexpr StringRequest::StringRequest(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : val_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string){}
struct StringRequestDefaultTypeInternal {
  constexpr StringRequestDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~StringRequestDefaultTypeInternal() {}
  union {
    StringRequest _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT StringRequestDefaultTypeInternal _StringRequest_default_instance_;
constexpr StringReply::StringReply(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : val_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string){}
struct StringReplyDefaultTypeInternal {
  constexpr StringReplyDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~StringReplyDefaultTypeInternal() {}
  union {
    StringReply _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT StringReplyDefaultTypeInternal _StringReply_default_instance_;
}  // namespace filesystemcomm
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_filesystemcomm_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_filesystemcomm_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_filesystemcomm_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_filesystemcomm_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::filesystemcomm::StringRequest, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::filesystemcomm::StringRequest, val_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::filesystemcomm::StringReply, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::filesystemcomm::StringReply, val_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::filesystemcomm::StringRequest)},
  { 7, -1, -1, sizeof(::filesystemcomm::StringReply)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::filesystemcomm::_StringRequest_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::filesystemcomm::_StringReply_default_instance_),
};

const char descriptor_table_protodef_filesystemcomm_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\024filesystemcomm.proto\022\016filesystemcomm\"\034"
  "\n\rStringRequest\022\013\n\003val\030\001 \001(\t\"\032\n\013StringRe"
  "ply\022\013\n\003val\030\001 \001(\t2`\n\021FileSystemService\022K\n"
  "\013SendRequest\022\035.filesystemcomm.StringRequ"
  "est\032\033.filesystemcomm.StringReply\"\000b\006prot"
  "o3"
  ;
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_filesystemcomm_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_filesystemcomm_2eproto = {
  false, false, 202, descriptor_table_protodef_filesystemcomm_2eproto, "filesystemcomm.proto", 
  &descriptor_table_filesystemcomm_2eproto_once, nullptr, 0, 2,
  schemas, file_default_instances, TableStruct_filesystemcomm_2eproto::offsets,
  file_level_metadata_filesystemcomm_2eproto, file_level_enum_descriptors_filesystemcomm_2eproto, file_level_service_descriptors_filesystemcomm_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_filesystemcomm_2eproto_getter() {
  return &descriptor_table_filesystemcomm_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_filesystemcomm_2eproto(&descriptor_table_filesystemcomm_2eproto);
namespace filesystemcomm {

// ===================================================================

class StringRequest::_Internal {
 public:
};

StringRequest::StringRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:filesystemcomm.StringRequest)
}
StringRequest::StringRequest(const StringRequest& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  val_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_val().empty()) {
    val_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_val(), 
      GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:filesystemcomm.StringRequest)
}

void StringRequest::SharedCtor() {
val_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

StringRequest::~StringRequest() {
  // @@protoc_insertion_point(destructor:filesystemcomm.StringRequest)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void StringRequest::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  val_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void StringRequest::ArenaDtor(void* object) {
  StringRequest* _this = reinterpret_cast< StringRequest* >(object);
  (void)_this;
}
void StringRequest::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void StringRequest::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void StringRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:filesystemcomm.StringRequest)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  val_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* StringRequest::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string val = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_val();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "filesystemcomm.StringRequest.val"));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* StringRequest::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:filesystemcomm.StringRequest)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string val = 1;
  if (!this->_internal_val().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_val().data(), static_cast<int>(this->_internal_val().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "filesystemcomm.StringRequest.val");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_val(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:filesystemcomm.StringRequest)
  return target;
}

size_t StringRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:filesystemcomm.StringRequest)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string val = 1;
  if (!this->_internal_val().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_val());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData StringRequest::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    StringRequest::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*StringRequest::GetClassData() const { return &_class_data_; }

void StringRequest::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<StringRequest *>(to)->MergeFrom(
      static_cast<const StringRequest &>(from));
}


void StringRequest::MergeFrom(const StringRequest& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:filesystemcomm.StringRequest)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_val().empty()) {
    _internal_set_val(from._internal_val());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void StringRequest::CopyFrom(const StringRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:filesystemcomm.StringRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool StringRequest::IsInitialized() const {
  return true;
}

void StringRequest::InternalSwap(StringRequest* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &val_, lhs_arena,
      &other->val_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata StringRequest::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_filesystemcomm_2eproto_getter, &descriptor_table_filesystemcomm_2eproto_once,
      file_level_metadata_filesystemcomm_2eproto[0]);
}

// ===================================================================

class StringReply::_Internal {
 public:
};

StringReply::StringReply(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:filesystemcomm.StringReply)
}
StringReply::StringReply(const StringReply& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  val_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_val().empty()) {
    val_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_val(), 
      GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:filesystemcomm.StringReply)
}

void StringReply::SharedCtor() {
val_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

StringReply::~StringReply() {
  // @@protoc_insertion_point(destructor:filesystemcomm.StringReply)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void StringReply::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  val_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void StringReply::ArenaDtor(void* object) {
  StringReply* _this = reinterpret_cast< StringReply* >(object);
  (void)_this;
}
void StringReply::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void StringReply::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void StringReply::Clear() {
// @@protoc_insertion_point(message_clear_start:filesystemcomm.StringReply)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  val_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* StringReply::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string val = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_val();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "filesystemcomm.StringReply.val"));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* StringReply::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:filesystemcomm.StringReply)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string val = 1;
  if (!this->_internal_val().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_val().data(), static_cast<int>(this->_internal_val().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "filesystemcomm.StringReply.val");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_val(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:filesystemcomm.StringReply)
  return target;
}

size_t StringReply::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:filesystemcomm.StringReply)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string val = 1;
  if (!this->_internal_val().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_val());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData StringReply::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    StringReply::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*StringReply::GetClassData() const { return &_class_data_; }

void StringReply::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<StringReply *>(to)->MergeFrom(
      static_cast<const StringReply &>(from));
}


void StringReply::MergeFrom(const StringReply& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:filesystemcomm.StringReply)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_val().empty()) {
    _internal_set_val(from._internal_val());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void StringReply::CopyFrom(const StringReply& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:filesystemcomm.StringReply)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool StringReply::IsInitialized() const {
  return true;
}

void StringReply::InternalSwap(StringReply* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &val_, lhs_arena,
      &other->val_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata StringReply::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_filesystemcomm_2eproto_getter, &descriptor_table_filesystemcomm_2eproto_once,
      file_level_metadata_filesystemcomm_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace filesystemcomm
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::filesystemcomm::StringRequest* Arena::CreateMaybeMessage< ::filesystemcomm::StringRequest >(Arena* arena) {
  return Arena::CreateMessageInternal< ::filesystemcomm::StringRequest >(arena);
}
template<> PROTOBUF_NOINLINE ::filesystemcomm::StringReply* Arena::CreateMaybeMessage< ::filesystemcomm::StringReply >(Arena* arena) {
  return Arena::CreateMessageInternal< ::filesystemcomm::StringReply >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
