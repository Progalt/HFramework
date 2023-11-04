
#include "Texture.h"
#include "FormatConvert.h"
#include "../Core/Log.h"
#include "TextureUtil.h"

namespace hf
{
	namespace vulkan
	{
        void Texture::Dispose()
        {
            if (!m_InternallyManaged)
            {
                vkDestroyImageView(m_AssociatedDevice, m_ImageView, nullptr);
                vmaDestroyImage(m_AssociatedAllocator, m_Image, m_Allocation);
            }
        }

        void Texture::Create(const TextureDesc& desc)
        {
            VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

            m_Format = FormatTable[(int)desc.format];

            if (desc.isRenderTarget)
            {
                if (IsColourFormat())
                {
                    usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                }
                else if (IsDepthFormat() || IsStencilFormat())
                {
                    usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                }
            }
            else
                usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = desc.width;
            imageInfo.extent.height = desc.height;
            imageInfo.extent.depth = desc.depth;
            imageInfo.mipLevels = desc.mipLevels;
            imageInfo.arrayLayers = desc.arrayLevels;
            imageInfo.format = m_Format;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usageFlags;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


            // For textures we create them on GPU only 
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            if (vmaCreateImage(m_AssociatedAllocator, &imageInfo, &allocCreateInfo, &m_Image, &m_Allocation, nullptr) != VK_SUCCESS)
            {
                Log::Fatal("Failed to create vulkan Image");
            }

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_Image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_Format;
            viewInfo.subresourceRange.aspectMask = GetAspectMask(this);
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = desc.mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = desc.arrayLevels;

            if (vkCreateImageView(m_AssociatedDevice, &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) 
            {
                Log::Fatal("Failed to create Image View");
            }

            m_Layout = imageInfo.initialLayout;
            m_Width = desc.width;
            m_Height = desc.height;
            m_Depth = desc.depth;
		}
	}
}