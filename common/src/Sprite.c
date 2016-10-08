#include "Sprite.h"
#include "Scroll.h"
#include "BankManager.h"

void InitSprite(struct Sprite* sprite, FrameSize size, UINT8 first_tile) {
	sprite->size = size;
	sprite->first_tile = first_tile;
	sprite->data = 0u;
	sprite->anim_speed = 33u;

	sprite->x = 0;
	sprite->y = 0;

	sprite->coll_x = 0u;
	sprite->coll_y = 0u;
	switch(size) {
		case FRAME_8x8:   sprite->coll_w =  8u; sprite->coll_h =  8u; break;
		case FRAME_8x16:  sprite->coll_w =  8u; sprite->coll_h = 16u; break;
		case FRAME_16x16: sprite->coll_w = 16u; sprite->coll_h = 16u; break;
		case FRAME_32x32: sprite->coll_w = 32u; sprite->coll_h = 32u; break;
	}

	sprite->custom_data_idx = 0;
}

void SetSpriteAnim(struct Sprite* sprite, UINT8* data, UINT8 speed) {
	if(sprite->data !=  data) {
		sprite->data = data;
		sprite->current_frame = 0;
		sprite->accum_ticks = 0;
		sprite->anim_speed = speed;
	}
}

extern UINT8 delta_time;
void DrawSprite(struct Sprite* sprite) {
	if(sprite->data) {
		
		sprite->accum_ticks += sprite->anim_speed << delta_time;
		if(sprite->accum_ticks > 100u) {
			sprite->current_frame ++;
			if(sprite->current_frame == sprite->data[0]){
				sprite->current_frame = 0;
			}

			sprite->accum_ticks -= 100u;
		}

		DrawFrame(sprite->oam_idx, sprite->size, sprite->first_tile + sprite->data[1 + sprite->current_frame], sprite->x, sprite->y, sprite->flags);
	}
}

UINT8 TranslateSprite(struct Sprite* sprite, INT8 x, INT8 y) {
	UINT16 start_x, start_y, n_its;
	unsigned char* tile;
	UINT8 i;
	UINT8 ret = 0;

	PUSH_BANK(scroll_bank);
	if(scroll_map) {
		if(x > 0) {
			start_x = (sprite->x + sprite->coll_x + sprite->coll_w + x);
			start_y = (sprite->y + sprite->coll_y);
			if(((start_y & 0xF000) | (start_x & 0xF000)) == 0u) {
				n_its = ((start_y + sprite->coll_h - 1u) >> 3) - (start_y >> 3) + 1u;
				tile = GetScrollTilePtr(start_x >> 3, start_y >> 3);
			
				for(i = 0u; i != n_its; ++i, tile += scroll_tiles_w) {
					if(scroll_collisions[*tile] == 1u) {
						x -= (start_x & (UINT16)7u);
						ret = *tile;
					}
				}
			}
		}
		if(x < 0) {
			start_x = (sprite->x + sprite->coll_x + (INT16)x);
			start_y = (sprite->y + sprite->coll_y);
			if(((start_y & 0xF000) | (start_x & 0xF000)) == 0u) {
				n_its = ((start_y + sprite->coll_h - 1u) >> 3) - (start_y >> 3) + 1u;
				tile = GetScrollTilePtr(start_x >> 3, start_y >> 3);
			
				for(i = 0u; i != n_its; ++i, tile += scroll_tiles_w) {
					if(scroll_collisions[*tile] == 1u) {
						x = (INT16)x + (8u - (start_x & (UINT16)7u));
						ret = *tile;
					}
				}
			}
		}
		if(y > 0) {
			start_x = (sprite->x + sprite->coll_x);
			start_y = (sprite->y + sprite->coll_y + sprite->coll_h + y);
			if(((start_y & 0xF000) | (start_x & 0xF000)) == 0u) {
				n_its = ((start_x + sprite->coll_w - 1u) >> 3) - (start_x >> 3) + 1u;
				tile = GetScrollTilePtr(start_x >> 3, start_y >> 3);
			
				for(i = 0u; i != n_its; ++i, tile += 1u) {
					if(scroll_collisions[*tile] == 1u || scroll_collisions_down[*tile] == 1u) {
						y -= (start_y & (UINT16)7u);
						ret = *tile;
					}
				}
			}
		}
		if(y < 0) {
			start_x = (sprite->x + sprite->coll_x);
			start_y = (sprite->y + sprite->coll_y + (INT16)y);
			if(((start_y & 0xF000) | (start_x & 0xF000)) == 0u) {
				n_its = ((start_x + sprite->coll_w - 1u) >> 3) - (start_x >> 3) + 1u;
				tile = GetScrollTilePtr(start_x >> 3, start_y >> 3);
			
				for(i = 0u; i != n_its; ++i, tile += 1u) {
					if(scroll_collisions[*tile] == 1u) {
						y = (INT16)y + (8u - (start_y & (UINT16)7u));
						ret = *tile;
					}
				}
			}
		}
	}
	POP_BANK;

	sprite->x += (INT16)x;
	sprite->y += (INT16)y;
	return ret;
}

UINT8 CheckCollision(struct Sprite* sprite1, struct Sprite* sprite2) {
	if( (sprite1->x + sprite1->coll_x > sprite2->x + sprite2->coll_x - sprite1->coll_w) &&
	    (sprite1->x + sprite1->coll_x < sprite2->x + sprite2->coll_x + sprite2->coll_w) &&
			(sprite1->y + sprite1->coll_y > sprite2->y + sprite2->coll_y - sprite1->coll_h) &&
			(sprite1->y + sprite1->coll_y < sprite2->y + sprite2->coll_y + sprite2->coll_h)
	) {
		return 1;
	} else {
		return 0;
	}
}