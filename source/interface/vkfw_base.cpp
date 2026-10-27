#include "interface/vkfw_base.h"

namespace Simple {
    namespace VKFW {

        bool shouldIgnoreCI(ContextIndex   _ci_check) { return _ci_check == VKFW_CI_IGNORE; }

        bool shouldIgnoreCI(ContextIndex2 _ci2_check) { return _ci2_check.first == VKFW_CI_IGNORE || _ci2_check.second == VKFW_CI_IGNORE; }

        namespace VulkanAPIVersion {

            uint32_t available(uint32_t *_variant) {
                uint32_t _version = VK_API_VERSION_1_0;

                if (vkEnumerateInstanceVersion != nullptr)
                    vkEnumerateInstanceVersion( &_version);

                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return _version & ~VK_MAKE_API_VERSION(0x7u, 0u, 0u, 0u); // variant bits are cleared
            }

            uint32_t join(const vec3u &_version, uint32_t _variant) {
                return VK_MAKE_API_VERSION(_variant, _version.x, _version.y, _version.z);
            }

            vec3u split(uint32_t _version, uint32_t *_variant) {
                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return { VK_API_VERSION_MAJOR(_version),
                         VK_API_VERSION_MINOR(_version),
                         VK_API_VERSION_PATCH(_version) };
            }
        }; // VulkanAPIVersion END


        std::vector<VkBaseOutStructure*> StructPtrChain::ExpandPtrChain(VkBaseOutStructure *_chain_begin) {
            std::vector<VkBaseOutStructure*> __res;

            while (_chain_begin != nullptr) {
                __res.push_back(_chain_begin);
                _chain_begin = _chain_begin->pNext;

                // Safeguard from loops
                for (uint32_t i = 0u; i < __res.size() - 1; ++i)
                    if (__res[i] == __res.back()) {
                        __res.pop_back();
                        _chain_begin = nullptr;
                        break;
                    }
            }

            return __res;
        }

        std::vector<VkStructureType> StructPtrChain::GetAllTypes(VkBaseOutStructure *_chain_begin) {
            auto __all_ptrs = ExpandPtrChain(_chain_begin);
            std::vector<VkStructureType> __res;

            __res.reserve(__all_ptrs.size());
            for (auto p : __all_ptrs) {
                // TODO: check if having multiple structures of same type is never allowed
                for (VkStructureType res_t : __res)
                    if (p->sType == res_t)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: StructPtrChain :: GetAllTypes", "structure chain has multiple structures of type " + std::to_string(res_t)));

                __res.push_back(p->sType);
            }
            return __res;
        }

        VkBaseOutStructure* StructPtrChain::GetChainElem(VkBaseOutStructure *_chain_begin, uint32_t _id) {
            auto __res = ExpandPtrChain(_chain_begin);
            return _id < __res.size() ? __res[_id] : nullptr;
        }

        VkBaseOutStructure* StructPtrChain::GetChainLastElem(VkBaseOutStructure *_chain_begin) {
            auto __res = ExpandPtrChain(_chain_begin);
            return __res.empty() ? nullptr : __res.back();
        }

        VkBaseOutStructure* StructPtrChain::InsertStructs(VkBaseOutStructure *_chain_begin, VkBaseOutStructure *_subchain_begin, uint32_t _offset) {
            if (_subchain_begin == nullptr || _chain_begin == nullptr)
                throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructPtrChain :: InsertStructs", "null pointer(s) passed"));

            VkBaseOutStructure *_subchain_end = GetChainLastElem(_subchain_begin);
            VkBaseOutStructure *_chain_before = nullptr;
            VkBaseOutStructure *_chain_after  = _chain_begin;

            if (_offset > 0u) {
                std::vector<VkBaseOutStructure*> __chain = ExpandPtrChain(_chain_begin);
                _chain_before = __chain[std::min<size_t>(_offset, __chain.size()) - 1];
                _chain_after  = _chain_before->pNext;
                _chain_before->pNext = _subchain_begin;
            }
            _subchain_end->pNext = _chain_after;
            return _offset > 0u ? _chain_begin : _subchain_begin;
        }

        StructPtrChain::StructPtrChain(VkBaseOutStructure *_chain_begin) : chain{ExpandPtrChain(_chain_begin)} {}

        StructPtrChain::StructPtrChain(const std::vector<void *> &_chain_vec) {
            chain.resize(_chain_vec.size());
            for (uint32_t i = 0; i < chain.size(); ++i)
                chain[i] = (VkBaseOutStructure *) _chain_vec[i];
            updateChainLinks();
        }

        StructPtrChain::StructPtrChain(StructPtrChain &&_chain) : chain{_chain.chain} { _chain.chain.clear(); }

        StructPtrChain::~StructPtrChain() { clear(); }

        void StructPtrChain::clear() {
            for (auto p : chain) ::operator delete(static_cast<void*>(p));
            chain.clear();
        }

        void StructPtrChain::updateChainLinks() {
            for (uint32_t i = 1u; i < chain.size(); ++i)
                chain[i-1]->pNext = chain[i];
            if (!chain.empty()) chain.back()->pNext = nullptr;
        }

        void StructPtrChain::setChain(VkBaseOutStructure *_chain_begin) {
            chain = ExpandPtrChain(_chain_begin);
        }

        std::vector<VkStructureType> StructPtrChain::getAllTypes() const {
            std::vector<VkStructureType> __res(chain.size());
            for (uint32_t i = 0u; i < __res.size(); ++i)
                __res[i] = chain[i]->sType;
            return __res;
        }

        void StructPtrChain::insertStructs(VkBaseOutStructure *_subchain_begin, uint32_t _offset) {
            auto __subchain = ExpandPtrChain(_subchain_begin);

            if (__subchain.empty())
                throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructPtrChain :: insertStructs", "subchain is empty"));

            auto __iter_begin = chain.end();
            if (_offset < chain.size())
                __iter_begin = chain.begin() + _offset;

            chain.insert(__iter_begin, __subchain.begin(), __subchain.end());
            updateChainLinks();
        }
    }; // VKFW END
}; // Simple END
