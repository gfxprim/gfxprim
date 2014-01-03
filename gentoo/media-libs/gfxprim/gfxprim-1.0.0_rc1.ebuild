# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit eutils

if [ "${PV}" = 9999 ]; then
	EGIT_REPO_URI="https://github.com/gfxprim/gfxprim.git"
	inherit git-2
	KEYWORDS=""
else
	SRC_URI="http://gfxprim.ucw.cz/releases/${P}.tar.bz2"
	KEYWORDS="~amd64 ~x86 ~arm ~ppc"
fi

DESCRIPTION="Modular 2D bitmap graphics library"
HOMEPAGE="http://gfxprim.ucw.cz/"

LICENSE="LGPL-2.1+"
SLOT="0"
IUSE="jpeg png gif tiff jpeg2k freetype X sdl aalib python"

RDEPEND="jpeg? ( virtual/jpeg )
	png? ( media-libs/libpng )
	gif? ( media-libs/giflib )
	tiff? ( media-libs/tiff )
	jpeg2k? ( >=media-libs/openjpeg-2.0.0 )
	freetype? ( media-libs/freetype )
	X? ( x11-libs/libX11 )
	sdl? ( media-libs/libsdl )
	aalib? ( media-libs/aalib )
"

DEPEND="$RDEPEND
	dev-python/jinja
	python? ( dev-lang/swig )
"

src_prepare() {
	if [ "${PV}" != 9999 ]; then
		epatch ${FILESDIR}/configure-more-compatible-with-autoconf.patch
	fi
}

src_configure() {
	use !jpeg && conf="${conf} --disable=jpeg"
	use !png && conf="${conf} --disable=libpng"
	use !gif && conf="${conf} --disable=giflib"
	use !tiff && conf="${conf} --disable=tiff"
	use !jpeg2k && conf="${conf} --disable=openjpeg"
	use !freetype && conf="${conf} --disable=freetype"
	use !X && conf="${conf} --disable=libX11"
	use !sdl && conf="${conf} --disable=libsdl"
	use !aalib && conf="${conf} --disable=aalib"
	use !python && conf="${conf} --PYTHON_CONFIG=''"

	econf ${conf}
}
