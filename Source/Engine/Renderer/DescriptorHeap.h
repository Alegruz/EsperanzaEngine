#pragma once

#include "Pch.h"

namespace esperanza
{
    // This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible
    // resource descriptors as resources are created.  For those that need to be made shader-visible, they
    // will need to be copied to a DescriptorHeap or a DynamicDescriptorHeap.
    class DescriptorAllocator final
    {
    public:
        static void DestroyAll(void);

    public:
        DescriptorAllocator() = delete;
        explicit DescriptorAllocator(_In_ ID3D12Device* pDevice, _In_ D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept;
        DescriptorAllocator(const DescriptorAllocator& other) = delete;
        DescriptorAllocator(DescriptorAllocator&& other) = delete;
        DescriptorAllocator& operator=(const DescriptorAllocator& other) = delete;
        DescriptorAllocator& operator=(DescriptorAllocator&& other) = delete;
        ~DescriptorAllocator() noexcept = default;

        HRESULT Allocate(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& outHandle, _In_ ID3D12Device* pDevice, _In_ uint32_t uCount);

    protected:
        static HRESULT requestNewHeap(_Out_ ID3D12DescriptorHeap** ppOutDescriptorHeap, _In_ ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type);

    protected:
        static constexpr const uint32_t NUM_DESCRIPTORS_PER_HEAP = 256;
        static std::mutex sm_AllocationMutex;
        static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool;

    protected:
        ComPtr<ID3D12Device> m_pDevice;
        D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
        ID3D12DescriptorHeap* m_pCurrentHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
        uint32_t m_uDescriptorSize;
        uint32_t m_uRemainingFreeHandles;
    };

    // This handle refers to a descriptor or a descriptor table (contiguous descriptors) that is shader visible.
    class DescriptorHandle
    {
    public:
        explicit DescriptorHandle() noexcept;
        explicit DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) noexcept;
        DescriptorHandle(const DescriptorHandle& other) noexcept = default;
        DescriptorHandle(DescriptorHandle&& other) noexcept = default;
        DescriptorHandle& operator=(const DescriptorHandle& other) noexcept = default;
        DescriptorHandle& operator=(DescriptorHandle&& other) noexcept = default;
        ~DescriptorHandle() noexcept = default;

        DescriptorHandle operator+(INT nOffsetScaledByDescriptorSize) const noexcept;

        void operator+=(INT nOffsetScaledByDescriptorSize) noexcept;

        constexpr const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const noexcept;
        constexpr operator D3D12_CPU_DESCRIPTOR_HANDLE() const noexcept;
        constexpr operator D3D12_GPU_DESCRIPTOR_HANDLE() const noexcept;

        constexpr size_t GetCpuPtr() const noexcept;
        constexpr uint64_t GetGpuPtr() const noexcept;
        constexpr bool IsNull() const noexcept;
        constexpr bool IsShaderVisible() const noexcept;

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
    };

    class DescriptorHeap
    {
    public:
        explicit DescriptorHeap() noexcept = default;
        DescriptorHeap(const DescriptorHeap& other) = delete;
        DescriptorHeap(DescriptorHeap&& other) = delete;
        DescriptorHeap& operator=(const DescriptorHeap& other) = delete;
        DescriptorHeap& operator=(DescriptorHeap&& other) = delete;
        ~DescriptorHeap() noexcept = default;

        HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strDebugHeapName, _In_ D3D12_DESCRIPTOR_HEAP_TYPE type, _In_ uint32_t uMaxCount) noexcept;
        void Destroy() noexcept;

        constexpr bool HasAvailableSpace(uint32_t uCount) const noexcept;
        DescriptorHandle Alloc() noexcept;
        DescriptorHandle Alloc(uint32_t uCount) noexcept;

        DescriptorHandle operator[] (uint32_t uArrayIdx) const noexcept;

        constexpr uint32_t GetOffsetOfHandle(const DescriptorHandle& dHandle) const noexcept;

        BOOL ValidateHandle(const DescriptorHandle& dHandle) const noexcept;

        ID3D12DescriptorHeap* GetHeapPointer() const noexcept;

        constexpr uint32_t GetDescriptorSize(void) const noexcept;

    private:
        ComPtr<ID3D12DescriptorHeap> m_pHeap;
        D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
        uint32_t m_uDescriptorSize;
        uint32_t m_uNumFreeDescriptors;
        DescriptorHandle m_FirstHandle;
        DescriptorHandle m_NextFreeHandle;
    };

    inline constexpr const D3D12_CPU_DESCRIPTOR_HANDLE* DescriptorHandle::operator&() const noexcept
    {
        return &m_CpuHandle;
    }

    inline constexpr DescriptorHandle::operator D3D12_CPU_DESCRIPTOR_HANDLE() const noexcept
    {
        return m_CpuHandle;
    }

    inline constexpr DescriptorHandle::operator D3D12_GPU_DESCRIPTOR_HANDLE() const noexcept
    {
        return m_GpuHandle;
    }

    inline constexpr size_t DescriptorHandle::GetCpuPtr() const noexcept
    {
        return m_CpuHandle.ptr;
    }

    inline constexpr uint64_t DescriptorHandle::GetGpuPtr() const noexcept
    {
        return m_GpuHandle.ptr;
    }

    inline constexpr bool DescriptorHandle::IsNull() const noexcept
    {
        return m_CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    inline constexpr bool DescriptorHandle::IsShaderVisible() const noexcept
    {
        return m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    inline constexpr bool DescriptorHeap::HasAvailableSpace(uint32_t uCount) const noexcept
    {
        return uCount <= m_uNumFreeDescriptors;
    }

    inline constexpr uint32_t DescriptorHeap::GetOffsetOfHandle(const DescriptorHandle& dHandle) const noexcept
    {
        return static_cast<uint32_t>((dHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr()) / m_uDescriptorSize);
    }

    inline constexpr uint32_t DescriptorHeap::GetDescriptorSize(void) const noexcept
    {
        return m_uDescriptorSize;
    }
}