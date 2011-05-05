Summary: VIA -- Volumetric Image Analysis
Name: via
Version: 1.5.0
Release: 0
License: GPL
Group: Applications/Engineering
BuildRoot: /var/tmp/%{name}-%{version}-build
Source: %{name}-%{version}.tar.gz

%description
Programs and libraries for volumeetric image analysis

%prep
%setup -q

%build
mkdir bin
mkdir libs
export VIA_HOME=/usr/src/packages/BUILD/via-1.5.0
export VIA_LIBS=$VIA_HOME/libs
export VIA_BIN=$VIA_HOME/bin
export VIA_INCLUDE=$VIA_HOME/include
export VIA_DOC=$VIA_HOME/doc
export VIA_MANUAL=$VIA_HOME/userdoc
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/include
mkdir -p $RPM_BUILD_ROOT/usr/include/via
mkdir -p $RPM_BUILD_ROOT/usr/include/viaio
mkdir -p $RPM_BUILD_ROOT/usr/share
mkdir -p $RPM_BUILD_ROOT/usr/share/X11
mkdir -p $RPM_BUILD_ROOT/usr/share/X11/app-defaults
mkdir -p $RPM_BUILD_ROOT/usr/share/man
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man3
mkdir -p $RPM_BUILD_ROOT/usr/share/doc
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via/html
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via/latex
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via-pgms
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via-pgms/html
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/via-pgms/latex
mkdir -p $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/etc/X11
mkdir -p $RPM_BUILD_ROOT/etc/X11/app-defaults

install -s -m 755 bin/*              $RPM_BUILD_ROOT/usr/bin/
install -m 755 via-help              $RPM_BUILD_ROOT/usr/bin/
install -m 755 via-doc               $RPM_BUILD_ROOT/usr/bin/
install -m 644 base/vxview/Vxview    $RPM_BUILD_ROOT/usr/share/X11/app-defaults/Vxview
install -m 644 base/vxview/Vxview    $RPM_BUILD_ROOT/etc/X11/app-defaults/Vxview
install -m 644 libs/*.a              $RPM_BUILD_ROOT/usr/lib/
install -m 644 include/*.h           $RPM_BUILD_ROOT/usr/include/via/
install -m 644 include/viaio/*.h     $RPM_BUILD_ROOT/usr/include/viaio/
install -m 644 doc/man/man3/*.3      $RPM_BUILD_ROOT/usr/share/man/man3/
install -m 644 doc/html/*.html	     $RPM_BUILD_ROOT/usr/share/doc/via/html/
install -m 644 doc/html/*.css	     $RPM_BUILD_ROOT/usr/share/doc/via/html/
install -m 644 doc/html/*.png	     $RPM_BUILD_ROOT/usr/share/doc/via/html/
install -m 644 doc/latex/*.tex       $RPM_BUILD_ROOT/usr/share/doc/via/latex/
install -m 644 doc/latex/*.pdf	     $RPM_BUILD_ROOT/usr/share/doc/via/latex/
install -m 644 doc/latex/*.sty       $RPM_BUILD_ROOT/usr/share/doc/via/latex/
install -m 644 doc/latex/*.tex	     $RPM_BUILD_ROOT/usr/share/doc/via/latex/
install -m 644 userdoc/man/man3/*.3  $RPM_BUILD_ROOT/usr/share/man/man1/
install -m 644 userdoc/html/*.html   $RPM_BUILD_ROOT/usr/share/doc/via-pgms/html/
install -m 644 userdoc/html/*.css    $RPM_BUILD_ROOT/usr/share/doc/via-pgms/html/
install -m 644 userdoc/html/*.png    $RPM_BUILD_ROOT/usr/share/doc/via-pgms/html/
install -m 644 userdoc/latex/*.tex   $RPM_BUILD_ROOT/usr/share/doc/via-pgms/latex/
install -m 644 userdoc/latex/*.sty   $RPM_BUILD_ROOT/usr/share/doc/via-pgms/latex/


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc Readme

   /etc/X11/app-defaults/Vxview
   /usr/bin/plaintov
   /usr/bin/pngtov
   /usr/bin/pnmtov
   /usr/bin/rawtov
   /usr/bin/vaniso2d
   /usr/bin/vaniso3d
   /usr/bin/vbinarize
   /usr/bin/vbinmorph3d
   /usr/bin/vbinsize
   /usr/bin/vcanny2d
   /usr/bin/vcanny3d
   /usr/bin/vcat
   /usr/bin/vcatbands
   /usr/bin/vcontrast
   /usr/bin/vconvert
   /usr/bin/vconvolve2d
   /usr/bin/vconvolve3d
   /usr/bin/vcrop
   /usr/bin/vcurvature
   /usr/bin/vdelsmall
   /usr/bin/vderiche3d
   /usr/bin/vdist3d
   /usr/bin/vflip
   /usr/bin/vgauss3d
   /usr/bin/vgenus3d
   /usr/bin/vgreymorph3d
   /usr/bin/vhemi
   /usr/bin/via-doc
   /usr/bin/via-help
   /usr/bin/vicp
   /usr/bin/vimage2graph
   /usr/bin/vimage2volumes
   /usr/bin/vimagehisto
   /usr/bin/vinvert
   /usr/bin/visodata
   /usr/bin/vistat
   /usr/bin/vkernel2d
   /usr/bin/vlabel2d
   /usr/bin/vlabel3d
   /usr/bin/vmedian3d
   /usr/bin/vnview
   /usr/bin/volumes2image
   /usr/bin/volumeselect
   /usr/bin/vop
   /usr/bin/vquickmorph3d
   /usr/bin/vrotate
   /usr/bin/vscale2d
   /usr/bin/vscale3d
   /usr/bin/vselbands
   /usr/bin/vselbig
   /usr/bin/vselect
   /usr/bin/vskel2d
   /usr/bin/vskel3d
   /usr/bin/vsmooth3d
   /usr/bin/vsynth
   /usr/bin/vthin3d
   /usr/bin/vtopgm
   /usr/bin/vtopnm
   /usr/bin/vtopoclass
   /usr/bin/vxview
   /usr/include/via/via.h
   /usr/include/via/viadata.h
   /usr/include/viaio/VEdges.h
   /usr/include/viaio/VGraph.h
   /usr/include/viaio/VImage.h
   /usr/include/viaio/VImageView.h
   /usr/include/viaio/VList.h
   /usr/include/viaio/VX.h
   /usr/include/viaio/VXPrivate.h
   /usr/include/viaio/Vlib.h
   /usr/include/viaio/Volumes.h
   /usr/include/viaio/colormap.h
   /usr/include/viaio/file.h
   /usr/include/viaio/headerinfo.h
   /usr/include/viaio/mu.h
   /usr/include/viaio/option.h
   /usr/include/viaio/os.h
   /usr/lib/libvia.a
   /usr/lib/libviaio.a
   /usr/lib/libvx.a
   /usr/share/X11/app-defaults/Vxview
   /usr/share/doc/via-pgms/html/doxygen.css
   /usr/share/doc/via-pgms/html/doxygen.png
   /usr/share/doc/via-pgms/html/files.html
   /usr/share/doc/via-pgms/html/index.html
   /usr/share/doc/via-pgms/html/plaintov_8c.html
   /usr/share/doc/via-pgms/html/pnmtov_8c.html
   /usr/share/doc/via-pgms/html/rawtov_8c.html
   /usr/share/doc/via-pgms/html/vaniso2d_8c.html
   /usr/share/doc/via-pgms/html/vaniso3d_8c.html
   /usr/share/doc/via-pgms/html/vbinarize_8c.html
   /usr/share/doc/via-pgms/html/vbinmorph3d_8c.html
   /usr/share/doc/via-pgms/html/vcanny2d_8c.html
   /usr/share/doc/via-pgms/html/vcanny3d_8c.html
   /usr/share/doc/via-pgms/html/vcat_8c.html
   /usr/share/doc/via-pgms/html/vcatbands_8c.html
   /usr/share/doc/via-pgms/html/vcontrast_8c.html
   /usr/share/doc/via-pgms/html/vconvert_8c.html
   /usr/share/doc/via-pgms/html/vconvolve2d_8c.html
   /usr/share/doc/via-pgms/html/vconvolve3d_8c.html
   /usr/share/doc/via-pgms/html/vcrop_8c.html
   /usr/share/doc/via-pgms/html/vcurvature_8c.html
   /usr/share/doc/via-pgms/html/vdelsmall_8c.html
   /usr/share/doc/via-pgms/html/vderiche3d_8c.html
   /usr/share/doc/via-pgms/html/vdist3d_8c.html
   /usr/share/doc/via-pgms/html/vflip_8c.html
   /usr/share/doc/via-pgms/html/vgauss3d_8c.html
   /usr/share/doc/via-pgms/html/vgenus3d_8c.html
   /usr/share/doc/via-pgms/html/vgreymorph3d_8c.html
   /usr/share/doc/via-pgms/html/vhemi_8c.html
   /usr/share/doc/via-pgms/html/vicp_8c.html
   /usr/share/doc/via-pgms/html/vimage2graph_8c.html
   /usr/share/doc/via-pgms/html/vimage2volumes_8c.html
   /usr/share/doc/via-pgms/html/vimagehisto_8c.html
   /usr/share/doc/via-pgms/html/vinvert_8c.html
   /usr/share/doc/via-pgms/html/visodata_8c.html
   /usr/share/doc/via-pgms/html/vistat_8c.html
   /usr/share/doc/via-pgms/html/vkernel2d_8c.html
   /usr/share/doc/via-pgms/html/vlabel2d_8c.html
   /usr/share/doc/via-pgms/html/vlabel3d_8c.html
   /usr/share/doc/via-pgms/html/vmedian3d_8c.html
   /usr/share/doc/via-pgms/html/volumes2image_8c.html
   /usr/share/doc/via-pgms/html/volumeselect_8c.html
   /usr/share/doc/via-pgms/html/vop_8c.html
   /usr/share/doc/via-pgms/html/vquickmorph3d_8c.html
   /usr/share/doc/via-pgms/html/vrotate_8c.html
   /usr/share/doc/via-pgms/html/vscale2d_8c.html
   /usr/share/doc/via-pgms/html/vscale3d_8c.html
   /usr/share/doc/via-pgms/html/vselbands_8c.html
   /usr/share/doc/via-pgms/html/vselbig_8c.html
   /usr/share/doc/via-pgms/html/vselect_8c.html
   /usr/share/doc/via-pgms/html/vskel2d_8c.html
   /usr/share/doc/via-pgms/html/vskel3d_8c.html
   /usr/share/doc/via-pgms/html/vsmooth3d_8c.html
   /usr/share/doc/via-pgms/html/vsynth_8c.html
   /usr/share/doc/via-pgms/html/vthin3d_8c.html
   /usr/share/doc/via-pgms/html/vtopgm_8c.html
   /usr/share/doc/via-pgms/html/vtopoclass_8c.html
   /usr/share/doc/via-pgms/latex/doxygen.sty
   /usr/share/doc/via-pgms/latex/files.tex
   /usr/share/doc/via-pgms/latex/plaintov_8c.tex
   /usr/share/doc/via-pgms/latex/pnmtov_8c.tex
   /usr/share/doc/via-pgms/latex/rawtov_8c.tex
   /usr/share/doc/via-pgms/latex/refman.tex
   /usr/share/doc/via-pgms/latex/vaniso2d_8c.tex
   /usr/share/doc/via-pgms/latex/vaniso3d_8c.tex
   /usr/share/doc/via-pgms/latex/vbinarize_8c.tex
   /usr/share/doc/via-pgms/latex/vbinmorph3d_8c.tex
   /usr/share/doc/via-pgms/latex/vcanny2d_8c.tex
   /usr/share/doc/via-pgms/latex/vcanny3d_8c.tex
   /usr/share/doc/via-pgms/latex/vcat_8c.tex
   /usr/share/doc/via-pgms/latex/vcatbands_8c.tex
   /usr/share/doc/via-pgms/latex/vcontrast_8c.tex
   /usr/share/doc/via-pgms/latex/vconvert_8c.tex
   /usr/share/doc/via-pgms/latex/vconvolve2d_8c.tex
   /usr/share/doc/via-pgms/latex/vconvolve3d_8c.tex
   /usr/share/doc/via-pgms/latex/vcrop_8c.tex
   /usr/share/doc/via-pgms/latex/vcurvature_8c.tex
   /usr/share/doc/via-pgms/latex/vdelsmall_8c.tex
   /usr/share/doc/via-pgms/latex/vderiche3d_8c.tex
   /usr/share/doc/via-pgms/latex/vdist3d_8c.tex
   /usr/share/doc/via-pgms/latex/vflip_8c.tex
   /usr/share/doc/via-pgms/latex/vgauss3d_8c.tex
   /usr/share/doc/via-pgms/latex/vgenus3d_8c.tex
   /usr/share/doc/via-pgms/latex/vgreymorph3d_8c.tex
   /usr/share/doc/via-pgms/latex/vhemi_8c.tex
   /usr/share/doc/via-pgms/latex/vicp_8c.tex
   /usr/share/doc/via-pgms/latex/vimage2graph_8c.tex
   /usr/share/doc/via-pgms/latex/vimage2volumes_8c.tex
   /usr/share/doc/via-pgms/latex/vimagehisto_8c.tex
   /usr/share/doc/via-pgms/latex/vinvert_8c.tex
   /usr/share/doc/via-pgms/latex/visodata_8c.tex
   /usr/share/doc/via-pgms/latex/vistat_8c.tex
   /usr/share/doc/via-pgms/latex/vkernel2d_8c.tex
   /usr/share/doc/via-pgms/latex/vlabel2d_8c.tex
   /usr/share/doc/via-pgms/latex/vlabel3d_8c.tex
   /usr/share/doc/via-pgms/latex/vmedian3d_8c.tex
   /usr/share/doc/via-pgms/latex/volumes2image_8c.tex
   /usr/share/doc/via-pgms/latex/volumeselect_8c.tex
   /usr/share/doc/via-pgms/latex/vop_8c.tex
   /usr/share/doc/via-pgms/latex/vquickmorph3d_8c.tex
   /usr/share/doc/via-pgms/latex/vrotate_8c.tex
   /usr/share/doc/via-pgms/latex/vscale2d_8c.tex
   /usr/share/doc/via-pgms/latex/vscale3d_8c.tex
   /usr/share/doc/via-pgms/latex/vselbands_8c.tex
   /usr/share/doc/via-pgms/latex/vselbig_8c.tex
   /usr/share/doc/via-pgms/latex/vselect_8c.tex
   /usr/share/doc/via-pgms/latex/vskel2d_8c.tex
   /usr/share/doc/via-pgms/latex/vskel3d_8c.tex
   /usr/share/doc/via-pgms/latex/vsmooth3d_8c.tex
   /usr/share/doc/via-pgms/latex/vsynth_8c.tex
   /usr/share/doc/via-pgms/latex/vthin3d_8c.tex
   /usr/share/doc/via-pgms/latex/vtopgm_8c.tex
   /usr/share/doc/via-pgms/latex/vtopoclass_8c.tex
   /usr/share/doc/via/html/doxygen.css
   /usr/share/doc/via/html/doxygen.png
   /usr/share/doc/via/html/Adjust_8c.html
   /usr/share/doc/via/html/Aniso2d_8c.html
   /usr/share/doc/via/html/Aniso3d_8c.html
   /usr/share/doc/via/html/Bicubic_8c.html
   /usr/share/doc/via/html/Binarize_8c.html
   /usr/share/doc/via/html/Binmorph3d_8c.html
   /usr/share/doc/via/html/Border3d_8c.html
   /usr/share/doc/via/html/BorderPoint_8c.html
   /usr/share/doc/via/html/CDT3d_8c.html
   /usr/share/doc/via/html/Canny3d_8c.html
   /usr/share/doc/via/html/Canny_8c.html
   /usr/share/doc/via/html/ChamferDist3d_8c.html
   /usr/share/doc/via/html/Contrast_8c.html
   /usr/share/doc/via/html/Convolve2d_8c.html
   /usr/share/doc/via/html/Convolve3d_8c.html
   /usr/share/doc/via/html/Convolve_8c.html
   /usr/share/doc/via/html/Curvature_8c.html
   /usr/share/doc/via/html/DeleteSmall_8c.html
   /usr/share/doc/via/html/Deriche2d_8c.html
   /usr/share/doc/via/html/Deriche3d_8c.html
   /usr/share/doc/via/html/Dist2d_8c.html
   /usr/share/doc/via/html/EuclideanDist3d_8c.html
   /usr/share/doc/via/html/Filter_8c.html
   /usr/share/doc/via/html/GaussConv2d_8c.html
   /usr/share/doc/via/html/GenusLee_8c.html
   /usr/share/doc/via/html/GraphPrune_8c.html
   /usr/share/doc/via/html/GreyMorph3d_8c.html
   /usr/share/doc/via/html/ICP_8c.html
   /usr/share/doc/via/html/Image2Graph_8c.html
   /usr/share/doc/via/html/Isodata_8c.html
   /usr/share/doc/via/html/Label2d_8c.html
   /usr/share/doc/via/html/Label3d_8c.html
   /usr/share/doc/via/html/Lee_8c.html
   /usr/share/doc/via/html/Magnitude_8c.html
   /usr/share/doc/via/html/Mask_8c.html
   /usr/share/doc/via/html/Median_8c.html
   /usr/share/doc/via/html/Moments_8c.html
   /usr/share/doc/via/html/NNSample3d_8c.html
   /usr/share/doc/via/html/NNScale3d_8c.html
   /usr/share/doc/via/html/NonmaxSuppression_8c.html
   /usr/share/doc/via/html/QuickMorph_8c.html
   /usr/share/doc/via/html/Rotate2d_8c.html
   /usr/share/doc/via/html/RotationMatrix_8c.html
   /usr/share/doc/via/html/Sample3d_8c.html
   /usr/share/doc/via/html/Scale2d_8c.html
   /usr/share/doc/via/html/Scale3d_8c.html
   /usr/share/doc/via/html/SelectBig_8c.html
   /usr/share/doc/via/html/ShapeMoments_8c.html
   /usr/share/doc/via/html/SimplePoint_8c.html
   /usr/share/doc/via/html/Skel2d_8c.html
   /usr/share/doc/via/html/Skel3d_8c.html
   /usr/share/doc/via/html/Smooth3d_8c.html
   /usr/share/doc/via/html/Spline_8c.html
   /usr/share/doc/via/html/Talairach_8c.html
   /usr/share/doc/via/html/Thin3d_8c.html
   /usr/share/doc/via/html/Topoclass_8c.html
   /usr/share/doc/via/html/VEdges_8h-source.html
   /usr/share/doc/via/html/VGraph_8h-source.html
   /usr/share/doc/via/html/VImageView_8h-source.html
   /usr/share/doc/via/html/VImage_8h-source.html
   /usr/share/doc/via/html/VList_8h-source.html
   /usr/share/doc/via/html/VXPrivate_8h-source.html
   /usr/share/doc/via/html/VX_8h-source.html
   /usr/share/doc/via/html/Vlib_8h-source.html
   /usr/share/doc/via/html/VolumesOps_8c.html
   /usr/share/doc/via/html/Volumes_8h-source.html
   /usr/share/doc/via/html/annotated.html
   /usr/share/doc/via/html/basin_8h-source.html
   /usr/share/doc/via/html/classes.html
   /usr/share/doc/via/html/colormap_8h-source.html
   /usr/share/doc/via/html/fftw_8h-source.html
   /usr/share/doc/via/html/file_8h-source.html
   /usr/share/doc/via/html/files.html
   /usr/share/doc/via/html/globals.html
   /usr/share/doc/via/html/graph_8h-source.html
   /usr/share/doc/via/html/headerinfo_8h-source.html
   /usr/share/doc/via/html/index.html
   /usr/share/doc/via/html/mu_8h-source.html
   /usr/share/doc/via/html/nr_8h-source.html
   /usr/share/doc/via/html/nrutil_8h-source.html
   /usr/share/doc/via/html/nrvia_8h-source.html
   /usr/share/doc/via/html/option_8h-source.html
   /usr/share/doc/via/html/os_8h-source.html
   /usr/share/doc/via/html/rfftw_8h-source.html
   /usr/share/doc/via/html/structFNode.html
   /usr/share/doc/via/html/structMyNodeStruct.html
   /usr/share/doc/via/html/structPixelList.html
   /usr/share/doc/via/html/structPoint.html
   /usr/share/doc/via/html/structPointList.html
   /usr/share/doc/via/html/structSEstruct.html
   /usr/share/doc/via/html/structSEstructure.html
   /usr/share/doc/via/html/structSNode.html
   /usr/share/doc/via/html/structVPoint.html
   /usr/share/doc/via/html/structVTrack.html
   /usr/share/doc/via/html/structValVoxel.html
   /usr/share/doc/via/html/structVolume.html
   /usr/share/doc/via/html/structVolumes.html
   /usr/share/doc/via/html/structVoxelList.html
   /usr/share/doc/via/html/structXPoint.html
   /usr/share/doc/via/html/via_8h-source.html
   /usr/share/doc/via/html/viadata_8h-source.html
   /usr/share/doc/via/latex/Adjust_8c.tex
   /usr/share/doc/via/latex/Aniso2d_8c.tex
   /usr/share/doc/via/latex/Aniso3d_8c.tex
   /usr/share/doc/via/latex/Bicubic_8c.tex
   /usr/share/doc/via/latex/Binarize_8c.tex
   /usr/share/doc/via/latex/Binmorph3d_8c.tex
   /usr/share/doc/via/latex/Border3d_8c.tex
   /usr/share/doc/via/latex/BorderPoint_8c.tex
   /usr/share/doc/via/latex/CDT3d_8c.tex
   /usr/share/doc/via/latex/Canny3d_8c.tex
   /usr/share/doc/via/latex/Canny_8c.tex
   /usr/share/doc/via/latex/ChamferDist3d_8c.tex
   /usr/share/doc/via/latex/Contrast_8c.tex
   /usr/share/doc/via/latex/Convolve2d_8c.tex
   /usr/share/doc/via/latex/Convolve3d_8c.tex
   /usr/share/doc/via/latex/Convolve_8c.tex
   /usr/share/doc/via/latex/Curvature_8c.tex
   /usr/share/doc/via/latex/DeleteSmall_8c.tex
   /usr/share/doc/via/latex/Deriche2d_8c.tex
   /usr/share/doc/via/latex/Deriche3d_8c.tex
   /usr/share/doc/via/latex/Dist2d_8c.tex
   /usr/share/doc/via/latex/EuclideanDist3d_8c.tex
   /usr/share/doc/via/latex/Filter_8c.tex
   /usr/share/doc/via/latex/GaussConv2d_8c.tex
   /usr/share/doc/via/latex/GenusLee_8c.tex
   /usr/share/doc/via/latex/GraphPrune_8c.tex
   /usr/share/doc/via/latex/GreyMorph3d_8c.tex
   /usr/share/doc/via/latex/ICP_8c.tex
   /usr/share/doc/via/latex/Image2Graph_8c.tex
   /usr/share/doc/via/latex/Isodata_8c.tex
   /usr/share/doc/via/latex/Label2d_8c.tex
   /usr/share/doc/via/latex/Label3d_8c.tex
   /usr/share/doc/via/latex/Lee_8c.tex
   /usr/share/doc/via/latex/Magnitude_8c.tex
   /usr/share/doc/via/latex/Mask_8c.tex
   /usr/share/doc/via/latex/Median_8c.tex
   /usr/share/doc/via/latex/Moments_8c.tex
   /usr/share/doc/via/latex/NNSample3d_8c.tex
   /usr/share/doc/via/latex/NNScale3d_8c.tex
   /usr/share/doc/via/latex/NonmaxSuppression_8c.tex
   /usr/share/doc/via/latex/QuickMorph_8c.tex
   /usr/share/doc/via/latex/Rotate2d_8c.tex
   /usr/share/doc/via/latex/RotationMatrix_8c.tex
   /usr/share/doc/via/latex/Sample3d_8c.tex
   /usr/share/doc/via/latex/Scale2d_8c.tex
   /usr/share/doc/via/latex/Scale3d_8c.tex
   /usr/share/doc/via/latex/SelectBig_8c.tex
   /usr/share/doc/via/latex/ShapeMoments_8c.tex
   /usr/share/doc/via/latex/SimplePoint_8c.tex
   /usr/share/doc/via/latex/Skel2d_8c.tex
   /usr/share/doc/via/latex/Skel3d_8c.tex
   /usr/share/doc/via/latex/Smooth3d_8c.tex
   /usr/share/doc/via/latex/Spline_8c.tex
   /usr/share/doc/via/latex/Talairach_8c.tex
   /usr/share/doc/via/latex/Thin3d_8c.tex
   /usr/share/doc/via/latex/Topoclass_8c.tex
   /usr/share/doc/via/latex/VolumesOps_8c.tex
   /usr/share/doc/via/latex/annotated.tex
   /usr/share/doc/via/latex/doxygen.sty
   /usr/share/doc/via/latex/files.tex
   /usr/share/doc/via/latex/refman.pdf
   /usr/share/doc/via/latex/refman.tex
   /usr/share/doc/via/latex/structFNode.tex
   /usr/share/doc/via/latex/structMyNodeStruct.tex
   /usr/share/doc/via/latex/structPixelList.tex
   /usr/share/doc/via/latex/structPoint.tex
   /usr/share/doc/via/latex/structPointList.tex
   /usr/share/doc/via/latex/structSEstruct.tex
   /usr/share/doc/via/latex/structSEstructure.tex
   /usr/share/doc/via/latex/structSNode.tex
   /usr/share/doc/via/latex/structVPoint.tex
   /usr/share/doc/via/latex/structVTrack.tex
   /usr/share/doc/via/latex/structValVoxel.tex
   /usr/share/doc/via/latex/structVolume.tex
   /usr/share/doc/via/latex/structVolumes.tex
   /usr/share/doc/via/latex/structVoxelList.tex
   /usr/share/doc/via/latex/structXPoint.tex
   /usr/share/man/man1/plaintov.c.3.gz
   /usr/share/man/man1/pnmtov.c.3.gz
   /usr/share/man/man1/rawtov.c.3.gz
   /usr/share/man/man1/vaniso2d.c.3.gz
   /usr/share/man/man1/vaniso3d.c.3.gz
   /usr/share/man/man1/vbinarize.c.3.gz
   /usr/share/man/man1/vbinmorph3d.c.3.gz
   /usr/share/man/man1/vcanny2d.c.3.gz
   /usr/share/man/man1/vcanny3d.c.3.gz
   /usr/share/man/man1/vcat.c.3.gz
   /usr/share/man/man1/vcatbands.c.3.gz
   /usr/share/man/man1/vcontrast.c.3.gz
   /usr/share/man/man1/vconvert.c.3.gz
   /usr/share/man/man1/vconvolve2d.c.3.gz
   /usr/share/man/man1/vconvolve3d.c.3.gz
   /usr/share/man/man1/vcrop.c.3.gz
   /usr/share/man/man1/vcurvature.c.3.gz
   /usr/share/man/man1/vdelsmall.c.3.gz
   /usr/share/man/man1/vderiche3d.c.3.gz
   /usr/share/man/man1/vdist3d.c.3.gz
   /usr/share/man/man1/vflip.c.3.gz
   /usr/share/man/man1/vgauss3d.c.3.gz
   /usr/share/man/man1/vgenus3d.c.3.gz
   /usr/share/man/man1/vgreymorph3d.c.3.gz
   /usr/share/man/man1/vhemi.c.3.gz
   /usr/share/man/man1/vicp.c.3.gz
   /usr/share/man/man1/vimage2graph.c.3.gz
   /usr/share/man/man1/vimage2volumes.c.3.gz
   /usr/share/man/man1/vimagehisto.c.3.gz
   /usr/share/man/man1/vinvert.c.3.gz
   /usr/share/man/man1/visodata.c.3.gz
   /usr/share/man/man1/vistat.c.3.gz
   /usr/share/man/man1/vkernel2d.c.3.gz
   /usr/share/man/man1/vlabel2d.c.3.gz
   /usr/share/man/man1/vlabel3d.c.3.gz
   /usr/share/man/man1/vmedian3d.c.3.gz
   /usr/share/man/man1/volumes2image.c.3.gz
   /usr/share/man/man1/volumeselect.c.3.gz
   /usr/share/man/man1/vop.c.3.gz
   /usr/share/man/man1/vquickmorph3d.c.3.gz
   /usr/share/man/man1/vrotate.c.3.gz
   /usr/share/man/man1/vscale2d.c.3.gz
   /usr/share/man/man1/vscale3d.c.3.gz
   /usr/share/man/man1/vselbands.c.3.gz
   /usr/share/man/man1/vselbig.c.3.gz
   /usr/share/man/man1/vselect.c.3.gz
   /usr/share/man/man1/vskel2d.c.3.gz
   /usr/share/man/man1/vskel3d.c.3.gz
   /usr/share/man/man1/vsmooth3d.c.3.gz
   /usr/share/man/man1/vsynth.c.3.gz
   /usr/share/man/man1/vthin3d.c.3.gz
   /usr/share/man/man1/vtopgm.c.3.gz
   /usr/share/man/man1/vtopoclass.c.3.gz
   /usr/share/man/man3/Adjust.c.3.gz
   /usr/share/man/man3/Aniso2d.c.3.gz
   /usr/share/man/man3/Aniso3d.c.3.gz
   /usr/share/man/man3/Bicubic.c.3.gz
   /usr/share/man/man3/Binarize.c.3.gz
   /usr/share/man/man3/Binmorph3d.c.3.gz
   /usr/share/man/man3/Border3d.c.3.gz
   /usr/share/man/man3/BorderPoint.c.3.gz
   /usr/share/man/man3/CDT3d.c.3.gz
   /usr/share/man/man3/Canny.c.3.gz
   /usr/share/man/man3/Canny3d.c.3.gz
   /usr/share/man/man3/ChamferDist3d.c.3.gz
   /usr/share/man/man3/Contrast.c.3.gz
   /usr/share/man/man3/Convolve.c.3.gz
   /usr/share/man/man3/Convolve2d.c.3.gz
   /usr/share/man/man3/Convolve3d.c.3.gz
   /usr/share/man/man3/Curvature.c.3.gz
   /usr/share/man/man3/DeleteSmall.c.3.gz
   /usr/share/man/man3/Deriche2d.c.3.gz
   /usr/share/man/man3/Deriche3d.c.3.gz
   /usr/share/man/man3/Dist2d.c.3.gz
   /usr/share/man/man3/EuclideanDist3d.c.3.gz
   /usr/share/man/man3/FNode.3.gz
   /usr/share/man/man3/Filter.c.3.gz
   /usr/share/man/man3/GaussConv2d.c.3.gz
   /usr/share/man/man3/GenusLee.c.3.gz
   /usr/share/man/man3/GraphPrune.c.3.gz
   /usr/share/man/man3/GreyMorph3d.c.3.gz
   /usr/share/man/man3/ICP.c.3.gz
   /usr/share/man/man3/Image2Graph.c.3.gz
   /usr/share/man/man3/Isodata.c.3.gz
   /usr/share/man/man3/Label2d.c.3.gz
   /usr/share/man/man3/Label3d.c.3.gz
   /usr/share/man/man3/Lee.c.3.gz
   /usr/share/man/man3/Magnitude.c.3.gz
   /usr/share/man/man3/Mask.c.3.gz
   /usr/share/man/man3/Median.c.3.gz
   /usr/share/man/man3/Moments.c.3.gz
   /usr/share/man/man3/MyNodeStruct.3.gz
   /usr/share/man/man3/NNSample3d.c.3.gz
   /usr/share/man/man3/NNScale3d.c.3.gz
   /usr/share/man/man3/NonmaxSuppression.c.3.gz
   /usr/share/man/man3/PixelList.3.gz
   /usr/share/man/man3/Point.3.gz
   /usr/share/man/man3/PointList.3.gz
   /usr/share/man/man3/QuickMorph.c.3.gz
   /usr/share/man/man3/Rotate2d.c.3.gz
   /usr/share/man/man3/RotationMatrix.c.3.gz
   /usr/share/man/man3/SEstruct.3.gz
   /usr/share/man/man3/SEstructure.3.gz
   /usr/share/man/man3/SNode.3.gz
   /usr/share/man/man3/Sample3d.c.3.gz
   /usr/share/man/man3/Scale2d.c.3.gz
   /usr/share/man/man3/Scale3d.c.3.gz
   /usr/share/man/man3/SelectBig.c.3.gz
   /usr/share/man/man3/ShapeMoments.c.3.gz
   /usr/share/man/man3/SimplePoint.c.3.gz
   /usr/share/man/man3/Skel2d.c.3.gz
   /usr/share/man/man3/Skel3d.c.3.gz
   /usr/share/man/man3/Smooth3d.c.3.gz
   /usr/share/man/man3/Spline.c.3.gz
   /usr/share/man/man3/Talairach.c.3.gz
   /usr/share/man/man3/Thin3d.c.3.gz
   /usr/share/man/man3/Topoclass.c.3.gz
   /usr/share/man/man3/VPoint.3.gz
   /usr/share/man/man3/VTrack.3.gz
   /usr/share/man/man3/ValVoxel.3.gz
   /usr/share/man/man3/Volume.3.gz
   /usr/share/man/man3/Volumes.3.gz
   /usr/share/man/man3/VolumesOps.c.3.gz
   /usr/share/man/man3/VoxelList.3.gz
