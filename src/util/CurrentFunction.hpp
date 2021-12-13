// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CURRENTFUNCTION_HPP
#define NOGGIT_CURRENTFUNCTION_HPP

#if defined( NOGGIT_DISABLE_CURRENT_FUNCTION )

  #define NOGGIT_CURRENT_FUNCTION "(unknown)"

#elif defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__) || defined(__clang__)

  #define NOGGIT_CURRENT_FUNCTION __PRETTY_FUNCTION__

#elif defined(__DMC__) && (__DMC__ >= 0x810)

  #define NOGGIT_CURRENT_FUNCTION __PRETTY_FUNCTION__

#elif defined(__FUNCSIG__)

  #define NOGGIT_CURRENT_FUNCTION __FUNCSIG__

#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))

  #define NOGGIT_CURRENT_FUNCTION __FUNCTION__

#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)

  #define NOGGIT_CURRENT_FUNCTION __FUNC__

#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)

  #define NOGGIT_CURRENT_FUNCTION __func__

#elif defined(__cplusplus) && (__cplusplus >= 201103)

  #define NOGGIT_CURRENT_FUNCTION __func__

#else

# define NOGGIT_CURRENT_FUNCTION "(unknown)"

#endif


#endif //NOGGIT_CURRENTFUNCTION_HPP
