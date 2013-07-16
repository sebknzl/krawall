/*
 * Krawall, XM/S3M Modplayer Library
 * Copyright (C) 2001-2005, 2013 Sebastian Kienzl
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License in COPYING for more details.
 */

inline static void volUp( MChannel *c, u32 v )
{
	c->volume += v;
	C_CLIP( c->volume, 64 );
	setChannelVol( c );
}

inline static void volDown( MChannel *c, u32 v )
{
	c->volume -= v;
	F_CLIP( c->volume, 0 );
	setChannelVol( c );
}

inline static void volSet( MChannel *c, u32 v )
{
	c->volume = v;
	C_CLIP( c->volume, 64 );
	setChannelVol( c );
}

inline static void panRight( MChannel *c, u32 p )
{
	c->panning += p;
	C_CLIP( c->panning, 64 );
	setChannelPan( c );
}

inline static void panLeft( MChannel *c, u32 p )
{
	c->panning -= p;
	F_CLIP( c->panning, -64 );
	setChannelPan( c );
}

inline static void panSet( MChannel *c, s32 p )
{
	c->panning = p;
	C_CLIP( c->panning, 64 );
	F_CLIP( c->panning, -64 );
	setChannelPan( c );
}

inline static int quickModulo( int a, int b )
{
	int ret;
	FastIntRem( a, b, &ret );
	return ret;
}

static void eff_speed( MChannel *c, bool onrow )
{
	if( c->effectop )
		player.speed = c->effectop;
}

static void eff_bpm( MChannel *c, bool onrow )
{
	if( c->effectop >= 0x20 )
		setTempo( c->effectop );
}

static void eff_speedbpm( MChannel *c, bool onrow )
{
	if( c->effectop >= 0x20 )
		setTempo( c->effectop );
	else
		player.speed = c->effectop;
}

static void eff_mark( MChannel *c, bool onrow )
{
	if( callBack )
		callBack( KRAP_CB_MARK, c->effectop );
}

static void eff_volslide_s3m( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->volslide = c->effectop;

		u8 h = c->volslide >> 4;
		u8 l = c->volslide & 0xf;
		c->volslidefine = false;

		if( h && ( l == 0xf ) ) {
			c->volslidefine = true;
			volUp( c, h );
		}
		else if( l && ( h == 0xf ) ) {
			c->volslidefine = true;
			volDown( c, l );
		}

		// don't reset note if last effect was vibrato
		if( c->effect == EFF_VOLSLIDE_VIBRATO &&
			c->resetnote &&
			c->leffect == EFF_VIBRATO )
			c->resetnote = false;
	}

	if( !onrow || player.fastVolSlides ) {
		if( c->volslidefine )
			return;

		u8 h = c->volslide >> 4;
		u8 l = c->volslide & 0xf;

		if( l ) {
			volDown( c, l );
		}
		else if( h ) {
			volUp( c, h );
		}
	}
}

static void eff_volslide_xm( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->volslide = c->effectop;

		// don't reset note if last effect was vibrato
		if( c->effect == EFF_VOLSLIDE_VIBRATO_XM &&
			c->resetnote &&
			c->leffect == EFF_VIBRATO )
			c->resetnote = false;
	}
	else {
		u8 h = c->volslide >> 4;
		u8 l = c->volslide & 0xf;

		if( l ) {
			volDown( c, l );
		}
		else if( h ) {
			volUp( c, h );
		}
	}
}

static void eff_volslide_df( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->volslide_fine = c->effectop;
	volDown( c, c->volslide_fine );
}


static void eff_volslide_uf( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->volslide_fine = c->effectop;
	volUp( c, c->volslide_fine );
}


static void eff_gvolume( MChannel *c, bool onrow )
{
	player.volume = c->effectop > 0x40 ? 0x40 : c->effectop;
    setGlobalVol();
}

static void eff_gvolslide( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			player.volSlide = c->effectop;
	}
	else {
		u8 h = player.volSlide >> 4;
		u8 l = player.volSlide & 0xf;

		if( l ) {
			player.volume -= l;
			F_CLIP( player.volume, 0 );
			setGlobalVol();
		}
		else if( h ) {
			player.volume += h;
			C_CLIP( player.volume, 0x40 );
			setGlobalVol();
		}
	}
}

static void eff_cvolume( MChannel *c, bool onrow )
{
	c->cvolume = c->effectop;
	C_CLIP( c->cvolume, 64 );
	setChannelVolH( c );
}

static void eff_cvolslide( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->cvolslide = c->effectop;
	}
	else {
		u8 h = c->cvolslide >> 4;
		u8 l = c->cvolslide & 0xf;

		if( l ) {
			c->cvolume -= l;
			F_CLIP( c->cvolume, 0 );
			setChannelVolH( c );
		}
		else if( h ) {
			c->cvolume += h;
			C_CLIP( c->cvolume, 0x40 );
			setChannelVolH( c );
		}
	}
}

static void eff_panslide( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->panslide = c->effectop;

		u8 h = c->panslide >> 4;
		u8 l = c->panslide & 0xf;
		c->panslidefine = false;

		if( h && ( l == 0xf ) ) {
			c->panslidefine = true;
			panLeft( c, h << 1 );
		}
		else if( l && ( h == 0xf ) ) {
			c->volslidefine = true;
			panRight( c, l << 1 );
		}
	}
	else {
		if( c->panslidefine )
			return;

		u8 h = c->panslide >> 4;
		u8 l = c->panslide & 0xf;

		if( l ) {
			panRight( c, l << 1 );
		}
		else if( h ) {
			panLeft( c, h << 1 );
		}
	}
}

static void eff_portaup_s3m( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->porta = c->effectop;

		if( c->porta >= 0xe0 ) {
			if( ( c->porta & 0xf0 ) == 0xf0 ) { // fine slide
				c->period -= ( c->porta & 0xf ) << 2;
			}
			if( ( c->porta & 0xf0 ) == 0xe0 ) { // extra fine slide
				c->period -= c->porta & 0xf;
			}

			if( c->period <= 0 ) {
				stopChannel( c );
			}

			setChannelFreq( c );
		}
	}
	else {
		if( c->porta && c->porta < 0xe0 ) {
			c->period -= ( c->porta << 2 );

			if( c->period <= 0 )
				stopChannel( c );
			else
				setChannelFreq( c );
		}
	}
}

/*
 XM-Portas:

 For up slides the porta-operand must be either added (linear) or substracted (Amiga).
 To avoid these distinctions the converter swaps UP with DOWN slides and vice versa
 when the module has Amiga slides enabled.
*/
static void eff_portaup_xm( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->porta = c->effectop;
	}
	else {
		c->period += ( c->porta << 2 );
		setChannelFreq( c );
	}
}

static void eff_portaup_f( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->porta_fine = c->effectop;
	c->period += ( c->porta_fine << 2 );
	setChannelFreq( c );
}

static void eff_portaup_ef( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->porta_efine = c->effectop;

	c->period += c->porta_efine;
	setChannelFreq( c );
}

static void eff_portadown_s3m( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->porta = c->effectop;

		if( c->porta >= 0xe0 ) {
			if( ( c->porta & 0xf0 ) == 0xf0 ) { // fine slide
				c->period += ( c->porta & 0xf ) << 2;
			}
			if( ( c->porta & 0xf0 ) == 0xe0 ) { // extra fine slide
				c->period += c->porta & 0xf;
			}

			setChannelFreq( c );
		}
	}
	else {
		if( c->porta && c->porta < 0xe0 ) {
			c->period += ( c->porta << 2 );
			setChannelFreq( c );
		}
	}
}

static void eff_portadown_xm( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->porta = c->effectop;
	}
	else {
		c->period -= ( c->porta << 2 );
		if( c->period <= 0 )
			stopChannel( c );
		else
			setChannelFreq( c );
	}
}

static void eff_portadown_f( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->porta_fine = c->effectop;

	c->period -= ( c->porta_fine << 2 );
	if( c->period <= 0 )
		stopChannel( c );
	else
		setChannelFreq( c );
}

static void eff_portadown_ef( MChannel *c, bool onrow )
{
	if( c->effectop )
		c->porta_efine = c->effectop;

	c->period -= c->porta_efine;
	if( c->period <= 0 )
		stopChannel( c );
	else
		setChannelFreq( c );
}


static void eff_portanote( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->portainc = c->effectop;
	}
	else {
		if( c->period > c->portadest ) { //slide up (linear: down)
			c->period -= ( c->portainc << 2 );
			F_CLIP( c->period, c->portadest );

			if( c->glissando ) {
				while( c->period < table_periods[ c->glissnote ] )
					c->glissnote++;
				if( c->period > table_periods[ c->glissnote ] )
					c->glissnote--;
			}
		}
		else {
			if( c->period < c->portadest ) { // slide down (linear: up)
				c->period += ( c->portainc << 2 );
				C_CLIP( c->period, c->portadest );

				if( c->glissando ) {
					while( c->period > table_periods[ c->glissnote ] )
						c->glissnote--;
					if( c->period < table_periods[ c->glissnote ] )
						c->glissnote++;
				}
			}
			else return;
		}

		if( !c->glissando )
			setChannelFreq( c );
		else // otherwise set current halftone!
			setChannelFreqH( c, table_periods[ c->glissnote ] );
	}
}


// inline for eff_fvibrato
static inline void eff_vibrato( MChannel *c, bool onrow )
{
	if( onrow ) {
		// retrig vibrato?
		if( c->playnote && c->vibrretrig )
			c->vibrpos = player.vibrOfs;

		// speed/depth seperately
		if( c->effectop & 0x0f )
			c->vibrdepth = ( c->effectop & 0xf ) << 2;
		if( c->effectop & 0xf0 )
			c->vibrspeed = c->effectop >> 4;

		// don't reset if last effect was volslide-vibrato
		if( c->resetnote && c->leffect == EFF_VOLSLIDE_VIBRATO )
			c->resetnote = false;
	}
	else {
		u16 vperiod = c->period + ( ( c->vibrdepth * c->vibrtable[ c->vibrpos ] ) >> 7 );
		setChannelFreqH( c, vperiod );
		c->vibrpos += c->vibrspeed;
		c->vibrpos &= ( TABLE_VIBRATO_LENGTH - 1 );
		c->resetnote = true;
	}
}

static void eff_fvibrato( MChannel *c, bool onrow )
{
	if( onrow ) {
		eff_vibrato( c, true );
		if( c->effectop & 0x0f )
			c->vibrdepth >>= 2;
	}
	else {
		eff_vibrato( c, false );
	}
}

static void eff_tremolo( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->playnote && c->tremretrig )
			c->trempos = 0;
		if( c->effectop & 0x0f )
			c->tremdepth = c->effectop & 0xf;
		if( c->effectop & 0xf0 )
			c->tremspeed = c->effectop >> 4;
	}
	else {
		s8 delta = ( c->tremdepth * c->tremtable[ c->trempos ] ) >> 6;
		c->volumec = c->volume + delta;
		F_CLIP( c->volumec, 0 );
		C_CLIP( c->volumec, 64 );
		setChannelVolH( c );
		c->trempos += c->tremspeed;
		c->trempos &= ( TABLE_VIBRATO_LENGTH - 1 );
		c->resetnote = true;
	}
}

static void eff_panbrello( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->playnote && c->panbretrig )
			c->panbpos = 0;
		if( c->effectop & 0x0f )
			c->panbdepth = c->effectop & 0xf;
		if( c->effectop & 0xf0 )
			c->panbspeed = c->effectop >> 4;
	}
	else {
		s8 delta = ( c->panbdepth * c->panbtable[ c->panbpos >> 2 ] ) >> 6;
		c->panningc = c->panning + delta;
		c->panbpos += c->panbspeed;
		c->panbpos &= ( TABLE_VIBRATO_LENGTH << 2 ) - 1;
		setChannelPanH( c );
		c->resetnote = true;
	}
}

static void eff_tremor( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop ) {
			c->trmon = ( c->effectop >> 4 ) + 1;
			c->trmtot = c->trmon + ( c->effectop & 0xf ) + 1;
		}
	}

	if( c->trmcnt == c->trmon ) {
		c->volumec = 0;
		setChannelVolH( c );
		c->resetnote = true;
	}
	else {
		if( c->trmcnt == c->trmtot ) {
			c->trmcnt = 0;
			setChannelVol( c );
			c->resetnote = false;
		}
	}

	c->trmcnt++;
}

static void eff_offset( MChannel *c, bool onrow )
{
	if( c->effectop ) {
		if( !c->playnote )
			setChannelPosH( c, c->effectop );
		else
			c->pos = c->effectop;
	}
}


static void eff_retrig( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->retrig = c->effectop;

		if( !c->playnote && c->retrig ) {
			setChannelPosH( c, 0 );
		}
    }
    else {
    	if( c->retrig && !quickModulo( player.tick, c->retrig & 0xf ) ) {
    		setChannelPosH( c, 0 );
			u8 volchg = c->retrig >> 4;
			if( volchg ) {
				if( volchg >=1 && volchg <= 5 ) {
					volDown( c, 1 << ( volchg - 1 ) );
				}
				else {
					if( volchg >= 9 && volchg <= 0xd ) {
						volUp( c, 1 << ( volchg - 9 ) );
					}
					else {
						switch( volchg ) {
						case 7:
							volSet( c, c->volume >> 1 );
							break;
						case 0xf:
							volSet( c, c->volume << 1 );
							break;
						case 0xe:
							volSet( c, ( c->volume * 3 ) >> 1 );
							break;
						case 6:
							// same as c->volume * 2/3 (exact as long as c->volume < 256)
							volSet( c, ( 342 * c->volume ) >> 9 );
							break;
						}
					}
				}
			}
    	}
	}
}

static void eff_arpeggio( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->arpeggio = c->effectop;

		if( !c->playnote ) { // base-tone!
			setChannelFreq( c );
		}

		c->arpeggiocnt = 0;
	}
	else {
		if( !c->sample )
			return;

		u16 note;
		if( ++c->arpeggiocnt >= 3 )
			c->arpeggiocnt = 0;

		switch( c->arpeggiocnt ) {
		case 0:
			note = c->note;
			break;
		case 1:
			note = c->note + ( c->arpeggio >> 4 );
			break;
		default:
			note = c->note + ( c->arpeggio & 0xf );
			break;
		}

		C_CLIP( note, TABLE_PERIODS_LENGTH - 1 );

		setChannelFreqNote( c, note );

		c->resetnote = true;
	}
}

static void eff_glissando( MChannel *c, bool onrow )
{
	c->glissando = ( c->effectop != 0 );
}

static void eff_wave_vibr( MChannel *c, bool onrow )
{
	switch( c->effectop & 3 ) {
	case 0:
		c->vibrtable = table_sine;
		break;
	case 1:
		c->vibrtable = table_ramp;
		break;
	case 2:
		c->vibrtable = table_square;
		break;
	case 3:
		c->vibrtable = table_rand;
		break;
	}

	if( c->effectop < 4 ) {
		c->vibrretrig = true;
	}
	else {
		c->vibrretrig = false;
	}
}

static void eff_wave_trem( MChannel *c, bool onrow )
{
	switch( c->effectop & 3 ) {
	case 0:
		c->tremtable = table_sine;
		break;
	case 1:
		c->tremtable = table_ramp;
		break;
	case 2:
		c->tremtable = table_square;
		break;
	case 3:
		c->tremtable = table_rand;
		break;
	}

	if( c->effectop < 4 ) {
		c->tremretrig = true;
	}
	else {
		c->tremretrig = false;
	}
}

static void eff_wave_panb( MChannel *c, bool onrow )
{
	switch( c->effectop & 3 ) {
	case 0:
		c->panbtable = table_sine;
		break;
	case 1:
		c->panbtable = table_ramp;
		break;
	case 2:
		c->panbtable = table_square;
		break;
	case 3:
		c->panbtable = table_rand;
		break;
	}

	if( c->effectop < 4 ) {
		c->panbretrig = true;
	}
	else {
		c->panbretrig = false;
	}
}

static void eff_pan( MChannel *c, bool onrow )
{
	if( c->effectop > 0x80 )
		c->panning = KRAM_SP_CENTER;
	else
		c->panning = c->effectop - 0x40;

	setChannelPan( c );
}
/*
static void eff_surround( MChannel *c, bool onrow )
{
	if( c->effectop ) {
		c->panning = KRAM_SP_SURROUND;
		setChannelPan( c );
	}
	else {
		c->panning = KRAM_SP_CENTER;
		setChannelPan( c );
	}
}
*/
static void eff_patt_jump( MChannel *c, bool onrow )
{
	player.pGotoOrder = c->effectop;
}

static void eff_patt_break( MChannel *c, bool onrow )
{
	player.pGotoRow = 10 * ( c->effectop >> 4 ) + ( c->effectop & 0xf );
}

static void eff_patternloop( MChannel *c, bool onrow )
{
	if( c->effectop ) {
		if( !player.pLoopActive ) {
			player.pLoopCount = c->effectop;
			player.pLoopActive = true;
		}

		if( !player.pLoopCount-- ) {
			player.pLoopActive = false;
		}
		else {
			player.pGotoRow = player.pLoopOrder;
		}
	}
	else {
		player.pLoopOrder = player.row;
	}
}

static void eff_notecut( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( !c->effectop )
			c->playnote = false;	// never play note (SC0)
	}
	else {
		if( c->effectop == player.tick ) {
			volSet( c, 0 );
		}
	}
}

static void eff_notedelay( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->playnote = false;
	}
	else {
		if( c->effectop == player.tick ) {
			playChannelNote( c );
		}
	}
}

static void eff_volume( MChannel *c, bool onrow )
{
	c->volume = c->effectop;
	setChannelVol( c );
}


// template:
/*
static void eff_( MChannel *c, bool onrow )
{
	if( onrow ) {
	}
	else {
	}
}
*/


//=============================================
//:: Volume-Column effects
//=============================================

static void eff_VC_volslide_down( MChannel *c, bool onrow )
{
	if( !onrow ) {
		volDown( c, c->effectVC & 0xf );
	}
}
static void eff_VC_volslide_up( MChannel *c, bool onrow )
{
	if( !onrow ) {
		volUp( c, c->effectVC & 0xf );
	}
}

static void eff_VC_fvolslide_down( MChannel *c, bool onrow )
{
	volDown( c, c->effectVC & 0xf );
}
static void eff_VC_fvolslide_up( MChannel *c, bool onrow )
{
	volUp( c, c->effectVC & 0xf );
}

static void eff_VC_vibrato_setspeed( MChannel *c, bool onrow )
{
	if( c->effectVC )
		c->vibrspeed = c->effectVC & 0xf;
}
static void eff_VC_vibrato( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectVC )
			c->vibrdepth = ( c->effectVC & 0xf ) << 2;
	}
	else {
		eff_vibrato( c, onrow );
	}
}

static void eff_VC_pan( MChannel *c, bool onrow )
{
	if( c->effectVC == 7 ) {
		c->panning = 0;
	}
	else {
		c->panning = ( int )( (4370*128*(c->effectVC & 0xf)) >> 16 ) - 64;
	}
	setChannelPan( c );
}
static void eff_VC_panslide_left( MChannel *c, bool onrow )
{
	if( !onrow ) {
		panLeft( c, c->effectVC & 0xf );
	}
}
static void eff_VC_panslide_right( MChannel *c, bool onrow )
{
	if( !onrow ) {
		panRight( c, c->effectVC & 0xf );
	}
}

static void eff_VC_portanote( MChannel *c, bool onrow )
{
	if( onrow ) {
		if( c->effectop )
			c->portainc = ( c->effectVC & 0xf ) << 4;
	}
	else {
		eff_portanote( c, true );
	}
}

