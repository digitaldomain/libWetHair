(use-modules
  ((guix licenses) #:prefix license:)
  (gnu packages algebra)
  (gnu packages compression)
  (gnu packages gl)
  (gnu packages image)
  (gnu packages pkg-config)
  (gnu packages stb)
  (gnu packages tbb)
  (gnu packages xorg)
  (guix build-system cmake)
  (guix build-system copy)
  (guix build-system gnu)
  (guix download)
  (guix gexp)
  (guix git-download)
  (guix packages)
  (guix utils)
  (ice-9 popen)
  (ice-9 rdelim)
  (ice-9 string-fun))

(define-public tclap
 (package
  (name "tclap")
  (version "1.2.5")
  (source
   (origin
     (method url-fetch)
     (uri (string-append "mirror://sourceforge/tclap/"
                         "tclap-" version ".tar.gz"))
     (sha256
      (base32 "19k564apa4mhlkvl66x8ws3xfqp0sk7jmdd4rc6qspp3v9v9yr5v"))))
  (build-system gnu-build-system)
  (home-page "http://tclap.sourceforge.net/v1.2/index.html")
  (synopsis "Command line argument parser written in ANSI C++")
  (description "Templatized C++ Command Line [argument] Parser library, is a small,
flexible C++ library that provides a simple interface for defining and
accessing command line arguments.")
  (license license:expat)))

(define-public rapidxml
  (package
    (name "rapidxml")
    (version "1.13")
    (source
     (origin
      (method url-fetch)
      (uri (string-append "mirror://sourceforge/rapidxml/"
                          "rapidxml-" version ".zip"))
      (sha256
       (base32 "0w9mbdgshr6sh6a5jr10lkdycjyvapbj7wxwz8hbp0a96y3biw63"))))
    (build-system copy-build-system)
    (arguments
     `(#:install-plan
       '(("rapidxml.hpp" "include/rapidxml/rapidxml.hpp")
         ("rapidxml_iterators.hpp" "include/rapidxml/rapidxml_iterators.hpp")
         ("rapidxml_print.hpp" "include/rapidxml/rapidxml_print.hpp")
         ("rapidxml_utils.hpp" "include/rapidxml/rapidxml_utils.hpp"))))
    (native-inputs (list unzip))
    (synopsis "A XML parser, written in modern C++.")
    (description "RapidXml is an attempt to create the fastest XML parser possible,
while retaining useability, portability and reasonable W3C
compatibility. It is an in-situ parser written in modern C++, with
parsing speed approaching that of strlen function executed on the same
data.")
    (home-page "http://rapidxml.sourceforge.net/")
    (license (list license:boost1.0 license:expat))))

(define-public anttweakbar
  (package
    (name "anttweakbar")
    (version "1.17")
    (source
    (origin
     (method git-fetch)
     (uri (git-reference
           (url "https://github.com/tschw/AntTweakBar.git")
           (commit "4ff73b96515164cbe66b484022e56e771ea6c224")))
     (sha256
      (base32
       "1yzd94agm2a8jwk1pgrcf0p8jd3my4il95d2s1493prdl1f5qmdq"))
     (file-name (git-file-name name version))))
    (build-system cmake-build-system)
    (arguments
     `(#:tests? #f))
    (native-inputs
     (list
      unzip))
    (inputs
     (list
      libx11
      mesa
      glu))
    (synopsis "GUI library written in C++.")
    (description "A small and easy-to-use C/C++ library to quickly add a light and
intuitive graphical user interface. Supports OpenGL and DirectX 9 to
11.")
    (home-page "http://anttweakbar.sourceforge.net/")
    (license license:zlib)))

(define %git-commit
  (read-string (open-pipe "git show HEAD | head -1 | cut -d ' ' -f2" OPEN_READ)))

(define (skip-git-directory file stat)
  "Skip the `.git` directory when collecting the sources."
  (let ((name (basename file)))
    (not (string=? name ".git"))))

(define-public libwethair-core
  (package
   (name "libwethair-core")
   (version (git-version "0" "HEAD" %git-commit))
   (source (local-file (dirname (current-filename))
                       #:recursive? #t
                       #:select? skip-git-directory))
   (build-system cmake-build-system)
   (arguments
    (list
     #:configure-flags
     #~(list
        "-DLIBWETHAIR_FIND_DEPENDENCIES=ON"
        "-DLIBWETHAIR_BUILD_APP=OFF"
        "-DLIBWETHAIR_INSTALL_ASSETS=OFF"
        "-DBUILD_SHARED_LIBS=YES"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
     #:tests? #f))
   (outputs '("out" "debug"))
   (native-inputs
    (list
     pkg-config))
   (inputs
    (list
     eigen
     tbb
     ))
   (home-page "https://www.cs.columbia.edu/cg/liquidhair/")
   (synopsis "A Multi-Scale Model for Simulating Liquid-Hair Interactions ")
   (description
    "Program for running simulations where liquid and hair interacts.")
   (license license:mpl2.0)))


(package
 (name "libwethair")
 (version (git-version "0" "HEAD" %git-commit))
 (source (local-file (dirname (current-filename))
                     #:recursive? #t
                     #:select? skip-git-directory))
 (build-system cmake-build-system)
 (arguments
  (list
   #:configure-flags
   #~(list
      (string-append "-DRAPIDXML_ROOT=" #$rapidxml)
      (string-append "-DSTB_IMAGE_WRITE_ROOT=" #$stb-image-write)
      "-DLIBWETHAIR_FIND_DEPENDENCIES=ON"
      "-DLIBWETHAIR_BUILD_CORE=OFF"
      "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
   #:tests? #f))
 (outputs '("out" "debug"))
 (native-inputs
  (list
   pkg-config))
 (inputs
  (list
   libwethair-core
   anttweakbar
   eigen
   glfw
   glew
   libpng
   rapidxml
   tbb
   tclap
   stb-image-write
   ))
 (home-page "https://www.cs.columbia.edu/cg/liquidhair/")
 (synopsis "A Multi-Scale Model for Simulating Liquid-Hair Interactions ")
 (description
  "Program for running simulations where liquid and hair interacts.")
 (license license:mpl2.0))
