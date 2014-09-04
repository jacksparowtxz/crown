/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "allocator.h"
#include "bundle.h"
#include "container_types.h"
#include "device.h"
#include "file.h"
#include "os.h"
#include "reader_writer.h"
#include "resource_manager.h"
#include "string_utils.h"
#include "types.h"
#include <cstring>
#include <inttypes.h>
#include <bgfx.h>

namespace crown
{

const uint32_t SPRITE_VERSION = 1;

struct SpriteHeader
{
	uint32_t version;
};

// header
// num_verts
// verts[num_verts]
// num_inds
// inds[num_inds]

struct SpriteResource
{
	const bgfx::Memory* vbmem;
	const bgfx::Memory* ibmem;
	bgfx::VertexBufferHandle vb;
	bgfx::IndexBufferHandle ib;
};

//-----------------------------------------------------------------------------
namespace sprite_resource
{
	void compile(crown::Filesystem&, char const*, crown::File*);
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
} // namespace sprite_resource

struct SpriteAnimationResource
{
	uint32_t version;
	uint32_t num_animations;
	uint32_t num_frames;
	uint32_t frames_offset;
};

struct SpriteAnimationName
{
	StringId32 id;
};

struct SpriteAnimationData
{
	uint32_t num_frames;
	uint32_t first_frame;
	float time;
};

namespace sprite_animation_resource
{
	void compile(crown::Filesystem&, char const*, crown::File*);
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
	const SpriteAnimationData* get_animation(const SpriteAnimationResource* sar, StringId32 name);
	const uint32_t* get_animation_frames(const SpriteAnimationResource* sar);
}

} // namespace crown