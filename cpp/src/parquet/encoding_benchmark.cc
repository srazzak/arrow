// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "benchmark/benchmark.h"

#include "arrow/array.h"
#include "arrow/array/builder_binary.h"
#include "arrow/array/builder_dict.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/random.h"
#include "arrow/testing/util.h"
#include "arrow/type.h"

#include "parquet/encoding.h"
#include "parquet/platform.h"
#include "parquet/schema.h"

#include <cmath>
#include <random>

using arrow::default_memory_pool;
using arrow::MemoryPool;

namespace {

// The min/max number of values used to drive each family of encoding benchmarks
constexpr int MIN_RANGE = 1024;
constexpr int MAX_RANGE = 65536;
}  // namespace

namespace parquet {

using schema::PrimitiveNode;

std::shared_ptr<ColumnDescriptor> Int64Schema(Repetition::type repetition) {
  auto node = PrimitiveNode::Make("int64", repetition, Type::INT64);
  return std::make_shared<ColumnDescriptor>(node, repetition != Repetition::REQUIRED,
                                            repetition == Repetition::REPEATED);
}

static void BM_PlainEncodingBoolean(benchmark::State& state) {
  std::vector<bool> values(state.range(0), true);
  auto encoder = MakeEncoder(Type::BOOLEAN, Encoding::PLAIN);
  auto typed_encoder = dynamic_cast<BooleanEncoder*>(encoder.get());

  for (auto _ : state) {
    typed_encoder->Put(values, static_cast<int>(values.size()));
    typed_encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(bool));
}

BENCHMARK(BM_PlainEncodingBoolean)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainDecodingBoolean(benchmark::State& state) {
  std::vector<bool> values(state.range(0), true);
  bool* output = new bool[state.range(0)];
  auto encoder = MakeEncoder(Type::BOOLEAN, Encoding::PLAIN);
  auto typed_encoder = dynamic_cast<BooleanEncoder*>(encoder.get());
  typed_encoder->Put(values, static_cast<int>(values.size()));
  std::shared_ptr<Buffer> buf = encoder->FlushValues();

  for (auto _ : state) {
    auto decoder = MakeTypedDecoder<BooleanType>(Encoding::PLAIN);
    decoder->SetData(static_cast<int>(values.size()), buf->data(),
                     static_cast<int>(buf->size()));
    decoder->Decode(output, static_cast<int>(values.size()));
  }

  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(bool));
  delete[] output;
}

BENCHMARK(BM_PlainDecodingBoolean)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainEncodingInt64(benchmark::State& state) {
  std::vector<int64_t> values(state.range(0), 64);
  auto encoder = MakeTypedEncoder<Int64Type>(Encoding::PLAIN);
  for (auto _ : state) {
    encoder->Put(values.data(), static_cast<int>(values.size()));
    encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(int64_t));
}

BENCHMARK(BM_PlainEncodingInt64)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainDecodingInt64(benchmark::State& state) {
  std::vector<int64_t> values(state.range(0), 64);
  auto encoder = MakeTypedEncoder<Int64Type>(Encoding::PLAIN);
  encoder->Put(values.data(), static_cast<int>(values.size()));
  std::shared_ptr<Buffer> buf = encoder->FlushValues();

  for (auto _ : state) {
    auto decoder = MakeTypedDecoder<Int64Type>(Encoding::PLAIN);
    decoder->SetData(static_cast<int>(values.size()), buf->data(),
                     static_cast<int>(buf->size()));
    decoder->Decode(values.data(), static_cast<int>(values.size()));
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(int64_t));
}

BENCHMARK(BM_PlainDecodingInt64)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainEncodingDouble(benchmark::State& state) {
  std::vector<double> values(state.range(0), 64.0);
  auto encoder = MakeTypedEncoder<DoubleType>(Encoding::PLAIN);
  for (auto _ : state) {
    encoder->Put(values.data(), static_cast<int>(values.size()));
    encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(double));
}

BENCHMARK(BM_PlainEncodingDouble)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainEncodingDoubleNaN(benchmark::State& state) {
  std::vector<double> values(state.range(0), nan(""));
  auto encoder = MakeTypedEncoder<DoubleType>(Encoding::PLAIN);
  for (auto _ : state) {
    encoder->Put(values.data(), static_cast<int>(values.size()));
    encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(double));
}

BENCHMARK(BM_PlainEncodingDoubleNaN)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainDecodingDouble(benchmark::State& state) {
  std::vector<double> values(state.range(0), 64.0);
  auto encoder = MakeTypedEncoder<DoubleType>(Encoding::PLAIN);
  encoder->Put(values.data(), static_cast<int>(values.size()));
  std::shared_ptr<Buffer> buf = encoder->FlushValues();

  for (auto _ : state) {
    auto decoder = MakeTypedDecoder<DoubleType>(Encoding::PLAIN);
    decoder->SetData(static_cast<int>(values.size()), buf->data(),
                     static_cast<int>(buf->size()));
    decoder->Decode(values.data(), static_cast<int>(values.size()));
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(double));
}

BENCHMARK(BM_PlainDecodingDouble)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainEncodingFloat(benchmark::State& state) {
  std::vector<float> values(state.range(0), 64.0);
  auto encoder = MakeTypedEncoder<FloatType>(Encoding::PLAIN);
  for (auto _ : state) {
    encoder->Put(values.data(), static_cast<int>(values.size()));
    encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(float));
}

BENCHMARK(BM_PlainEncodingFloat)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainEncodingFloatNaN(benchmark::State& state) {
  std::vector<float> values(state.range(0), nanf(""));
  auto encoder = MakeTypedEncoder<FloatType>(Encoding::PLAIN);
  for (auto _ : state) {
    encoder->Put(values.data(), static_cast<int>(values.size()));
    encoder->FlushValues();
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(float));
}

BENCHMARK(BM_PlainEncodingFloatNaN)->Range(MIN_RANGE, MAX_RANGE);

static void BM_PlainDecodingFloat(benchmark::State& state) {
  std::vector<float> values(state.range(0), 64.0);
  auto encoder = MakeTypedEncoder<FloatType>(Encoding::PLAIN);
  encoder->Put(values.data(), static_cast<int>(values.size()));
  std::shared_ptr<Buffer> buf = encoder->FlushValues();

  for (auto _ : state) {
    auto decoder = MakeTypedDecoder<FloatType>(Encoding::PLAIN);
    decoder->SetData(static_cast<int>(values.size()), buf->data(),
                     static_cast<int>(buf->size()));
    decoder->Decode(values.data(), static_cast<int>(values.size()));
  }
  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(float));
}

BENCHMARK(BM_PlainDecodingFloat)->Range(MIN_RANGE, MAX_RANGE);

template <typename Type>
static void DecodeDict(std::vector<typename Type::c_type>& values,
                       benchmark::State& state) {
  typedef typename Type::c_type T;
  int num_values = static_cast<int>(values.size());

  MemoryPool* allocator = default_memory_pool();
  std::shared_ptr<ColumnDescriptor> descr = Int64Schema(Repetition::REQUIRED);

  auto base_encoder =
      MakeEncoder(Type::type_num, Encoding::PLAIN, true, descr.get(), allocator);
  auto encoder =
      dynamic_cast<typename EncodingTraits<Type>::Encoder*>(base_encoder.get());
  auto dict_traits = dynamic_cast<DictEncoder<Type>*>(base_encoder.get());
  encoder->Put(values.data(), num_values);

  std::shared_ptr<ResizableBuffer> dict_buffer =
      AllocateBuffer(allocator, dict_traits->dict_encoded_size());

  std::shared_ptr<ResizableBuffer> indices =
      AllocateBuffer(allocator, encoder->EstimatedDataEncodedSize());

  dict_traits->WriteDict(dict_buffer->mutable_data());
  int actual_bytes = dict_traits->WriteIndices(indices->mutable_data(),
                                               static_cast<int>(indices->size()));

  PARQUET_THROW_NOT_OK(indices->Resize(actual_bytes));

  for (auto _ : state) {
    auto dict_decoder = MakeTypedDecoder<Type>(Encoding::PLAIN, descr.get());
    dict_decoder->SetData(dict_traits->num_entries(), dict_buffer->data(),
                          static_cast<int>(dict_buffer->size()));

    auto decoder = MakeDictDecoder<Type>(descr.get());
    decoder->SetDict(dict_decoder.get());
    decoder->SetData(num_values, indices->data(), static_cast<int>(indices->size()));
    decoder->Decode(values.data(), num_values);
  }

  state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(T));
}

static void BM_DictDecodingInt64_repeats(benchmark::State& state) {
  typedef Int64Type Type;
  typedef typename Type::c_type T;

  std::vector<T> values(state.range(0), 64);
  DecodeDict<Type>(values, state);
}

BENCHMARK(BM_DictDecodingInt64_repeats)->Range(MIN_RANGE, MAX_RANGE);

static void BM_DictDecodingInt64_literals(benchmark::State& state) {
  typedef Int64Type Type;
  typedef typename Type::c_type T;

  std::vector<T> values(state.range(0));
  for (size_t i = 0; i < values.size(); ++i) {
    values[i] = i;
  }
  DecodeDict<Type>(values, state);
}

BENCHMARK(BM_DictDecodingInt64_literals)->Range(MIN_RANGE, MAX_RANGE);

// ----------------------------------------------------------------------
// Shared benchmarks for decoding using arrow builders
class BenchmarkDecodeArrow : public ::benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) override {
    num_values_ = static_cast<int>(state.range());
    InitDataInputs();
    DoEncodeArrow();
  }

  void TearDown(const ::benchmark::State& state) override {
    buffer_.reset();
    input_array_.reset();
  }

  void InitDataInputs() {
    // Generate a random string dictionary without any nulls so that this dataset can be
    // used for benchmarking the DecodeArrowNonNull API
    constexpr int repeat_factor = 8;
    constexpr int64_t min_length = 2;
    constexpr int64_t max_length = 10;
    ::arrow::random::RandomArrayGenerator rag(0);
    input_array_ = rag.StringWithRepeats(num_values_, num_values_ / repeat_factor,
                                         min_length, max_length, /*null_probability=*/0);
    valid_bits_ = input_array_->null_bitmap()->data();
    total_size_ = input_array_->data()->buffers[2]->size();

    values_.reserve(num_values_);
    const auto& binary_array = static_cast<const ::arrow::BinaryArray&>(*input_array_);
    for (int64_t i = 0; i < binary_array.length(); i++) {
      auto view = binary_array.GetView(i);
      values_.emplace_back(static_cast<uint32_t>(view.length()),
                           reinterpret_cast<const uint8_t*>(view.data()));
    }
  }

  virtual void DoEncodeArrow() = 0;
  virtual void DoEncodeLowLevel() = 0;

  virtual std::unique_ptr<ByteArrayDecoder> InitializeDecoder() = 0;

  template <typename BuilderType>
  std::unique_ptr<BuilderType> CreateBuilder();

  void EncodeArrowBenchmark(benchmark::State& state) {
    for (auto _ : state) {
      DoEncodeArrow();
    }
    state.SetBytesProcessed(state.iterations() * total_size_);
  }

  void EncodeLowLevelBenchmark(benchmark::State& state) {
    for (auto _ : state) {
      DoEncodeLowLevel();
    }
    state.SetBytesProcessed(state.iterations() * total_size_);
  }

  template <typename BuilderType>
  void DecodeArrowBenchmark(benchmark::State& state) {
    for (auto _ : state) {
      auto decoder = InitializeDecoder();
      auto builder = CreateBuilder<BuilderType>();
      decoder->DecodeArrow(num_values_, 0, valid_bits_, 0, builder.get());
    }

    state.SetBytesProcessed(state.iterations() * total_size_);
  }

  template <typename BuilderType>
  void DecodeArrowNonNullBenchmark(benchmark::State& state) {
    for (auto _ : state) {
      auto decoder = InitializeDecoder();
      auto builder = CreateBuilder<BuilderType>();
      decoder->DecodeArrowNonNull(num_values_, builder.get());
    }

    state.SetBytesProcessed(state.iterations() * total_size_);
  }

 protected:
  int num_values_;
  std::shared_ptr<::arrow::Array> input_array_;
  std::vector<ByteArray> values_;
  uint64_t total_size_;
  const uint8_t* valid_bits_;
  std::shared_ptr<Buffer> buffer_;
};

using ::arrow::BinaryDictionary32Builder;
using ::arrow::internal::ChunkedBinaryBuilder;

template <>
std::unique_ptr<ChunkedBinaryBuilder> BenchmarkDecodeArrow::CreateBuilder() {
  int chunk_size = static_cast<int>(buffer_->size());
  return std::unique_ptr<ChunkedBinaryBuilder>(
      new ChunkedBinaryBuilder(chunk_size, default_memory_pool()));
}

template <>
std::unique_ptr<BinaryDictionary32Builder> BenchmarkDecodeArrow::CreateBuilder() {
  return std::unique_ptr<BinaryDictionary32Builder>(
      new BinaryDictionary32Builder(default_memory_pool()));
}

// ----------------------------------------------------------------------
// Benchmark Decoding from Plain Encoding
class BM_ArrowBinaryPlain : public BenchmarkDecodeArrow {
 public:
  void DoEncodeArrow() override {
    auto encoder = MakeTypedEncoder<ByteArrayType>(Encoding::PLAIN);
    encoder->Put(*input_array_);
    buffer_ = encoder->FlushValues();
  }

  void DoEncodeLowLevel() override {
    auto encoder = MakeTypedEncoder<ByteArrayType>(Encoding::PLAIN);
    encoder->Put(values_.data(), num_values_);
    buffer_ = encoder->FlushValues();
  }

  std::unique_ptr<ByteArrayDecoder> InitializeDecoder() override {
    auto decoder = MakeTypedDecoder<ByteArrayType>(Encoding::PLAIN);
    decoder->SetData(num_values_, buffer_->data(), static_cast<int>(buffer_->size()));
    return decoder;
  }
};

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, EncodeArrow)
(benchmark::State& state) { EncodeArrowBenchmark(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, EncodeArrow)->Range(1 << 18, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, EncodeLowLevel)
(benchmark::State& state) { EncodeLowLevelBenchmark(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, EncodeLowLevel)->Range(1 << 18, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, DecodeArrow_Dense)
(benchmark::State& state) { DecodeArrowBenchmark<ChunkedBinaryBuilder>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, DecodeArrow_Dense)->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, DecodeArrowNonNull_Dense)
(benchmark::State& state) { DecodeArrowNonNullBenchmark<ChunkedBinaryBuilder>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, DecodeArrowNonNull_Dense)
    ->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, DecodeArrow_Dict)
(benchmark::State& state) { DecodeArrowBenchmark<BinaryDictionary32Builder>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, DecodeArrow_Dict)->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryPlain, DecodeArrowNonNull_Dict)
(benchmark::State& state) {
  DecodeArrowNonNullBenchmark<BinaryDictionary32Builder>(state);
}
BENCHMARK_REGISTER_F(BM_ArrowBinaryPlain, DecodeArrowNonNull_Dict)
    ->Range(MIN_RANGE, MAX_RANGE);

// ----------------------------------------------------------------------
// Benchmark Decoding from Dictionary Encoding
class BM_ArrowBinaryDict : public BenchmarkDecodeArrow {
 public:
  template <typename PutValuesFunc>
  void DoEncode(PutValuesFunc&& put_values) {
    auto node = schema::ByteArray("name");
    descr_ = std::unique_ptr<ColumnDescriptor>(new ColumnDescriptor(node, 0, 0));

    auto encoder = MakeTypedEncoder<ByteArrayType>(Encoding::PLAIN,
                                                   /*use_dictionary=*/true, descr_.get());
    put_values(encoder.get());
    buffer_ = encoder->FlushValues();

    auto dict_encoder = dynamic_cast<DictEncoder<ByteArrayType>*>(encoder.get());
    ASSERT_NE(dict_encoder, nullptr);
    dict_buffer_ =
        AllocateBuffer(default_memory_pool(), dict_encoder->dict_encoded_size());
    dict_encoder->WriteDict(dict_buffer_->mutable_data());
    num_dict_entries_ = dict_encoder->num_entries();
  }

  template <typename IndexType>
  void EncodeDictBenchmark(benchmark::State& state) {
    constexpr int64_t nunique = 100;
    constexpr int64_t min_length = 32;
    constexpr int64_t max_length = 32;
    ::arrow::random::RandomArrayGenerator rag(0);
    auto dict = rag.String(nunique, min_length, max_length,
                           /*null_probability=*/0);
    auto indices = rag.Numeric<IndexType, int32_t>(num_values_, 0, nunique - 1);

    auto PutValues = [&](ByteArrayEncoder* encoder) {
      auto dict_encoder = dynamic_cast<DictEncoder<ByteArrayType>*>(encoder);
      dict_encoder->PutDictionary(*dict);
      dict_encoder->PutIndices(*indices);
    };
    for (auto _ : state) {
      DoEncode(std::move(PutValues));
    }
    state.SetItemsProcessed(state.iterations() * num_values_);
  }

  void DoEncodeArrow() override {
    auto PutValues = [&](ByteArrayEncoder* encoder) {
      ASSERT_NO_THROW(encoder->Put(*input_array_));
    };
    DoEncode(std::move(PutValues));
  }

  void DoEncodeLowLevel() override {
    auto PutValues = [&](ByteArrayEncoder* encoder) {
      encoder->Put(values_.data(), num_values_);
    };
    DoEncode(std::move(PutValues));
  }

  std::unique_ptr<ByteArrayDecoder> InitializeDecoder() override {
    auto decoder = MakeTypedDecoder<ByteArrayType>(Encoding::PLAIN, descr_.get());
    decoder->SetData(num_dict_entries_, dict_buffer_->data(),
                     static_cast<int>(dict_buffer_->size()));
    auto dict_decoder = MakeDictDecoder<ByteArrayType>(descr_.get());
    dict_decoder->SetDict(decoder.get());
    dict_decoder->SetData(num_values_, buffer_->data(),
                          static_cast<int>(buffer_->size()));
    return std::unique_ptr<ByteArrayDecoder>(
        dynamic_cast<ByteArrayDecoder*>(dict_decoder.release()));
  }

  void TearDown(const ::benchmark::State& state) override {
    BenchmarkDecodeArrow::TearDown(state);
    dict_buffer_.reset();
    descr_.reset();
  }

 protected:
  std::unique_ptr<ColumnDescriptor> descr_;
  std::shared_ptr<Buffer> dict_buffer_;
  int num_dict_entries_;
};

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeArrow)
(benchmark::State& state) { EncodeArrowBenchmark(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeArrow)->Range(1 << 18, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeDictDirectInt8)
(benchmark::State& state) { EncodeDictBenchmark<::arrow::Int8Type>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeDictDirectInt8)->Range(1 << 20, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeDictDirectInt16)
(benchmark::State& state) { EncodeDictBenchmark<::arrow::Int16Type>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeDictDirectInt16)->Range(1 << 20, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeDictDirectInt32)
(benchmark::State& state) { EncodeDictBenchmark<::arrow::Int32Type>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeDictDirectInt32)->Range(1 << 20, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeDictDirectInt64)
(benchmark::State& state) { EncodeDictBenchmark<::arrow::Int64Type>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeDictDirectInt64)->Range(1 << 20, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, EncodeLowLevel)
(benchmark::State& state) { EncodeLowLevelBenchmark(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, EncodeLowLevel)->Range(1 << 18, 1 << 20);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, DecodeArrow_Dense)(benchmark::State& state) {
  DecodeArrowBenchmark<ChunkedBinaryBuilder>(state);
}
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, DecodeArrow_Dense)->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, DecodeArrowNonNull_Dense)
(benchmark::State& state) { DecodeArrowNonNullBenchmark<ChunkedBinaryBuilder>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, DecodeArrowNonNull_Dense)
    ->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, DecodeArrow_Dict)
(benchmark::State& state) { DecodeArrowBenchmark<BinaryDictionary32Builder>(state); }
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, DecodeArrow_Dict)->Range(MIN_RANGE, MAX_RANGE);

BENCHMARK_DEFINE_F(BM_ArrowBinaryDict, DecodeArrowNonNull_Dict)
(benchmark::State& state) {
  DecodeArrowNonNullBenchmark<BinaryDictionary32Builder>(state);
}
BENCHMARK_REGISTER_F(BM_ArrowBinaryDict, DecodeArrowNonNull_Dict)
    ->Range(MIN_RANGE, MAX_RANGE);

}  // namespace parquet
