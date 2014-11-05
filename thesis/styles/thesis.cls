%==============================================================================%
% Identification
%==============================================================================%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{thesis}

%==============================================================================%
% Preliminary declarations
%==============================================================================%
% Declare boolean variable standard
\newif\ifstandard

%==============================================================================%
% Options
%==============================================================================%
% Option standard: \doumentclass[standard]{thesis}
\DeclareOption{standard}{
  \standardtrue 
}
% Option koma: \doumentclass[koma]{thesis}
\DeclareOption{koma}{
  \standardfalse
}
% Default if no options given: \doumentclass{thesis}
\ExecuteOptions{standard}

% Execute the code of each option above
\ProcessOptions\relax

%==============================================================================%
% Main body
%==============================================================================%
\ifstandard
  \LoadClass[a4paper]{report}
  \RequirePackage{a4wide}
\else
  \LoadClass{scrreprt}
  \setcapindent{2em} % Indentation in Figure captions
\fi

\RequirePackage[utf8]{inputenc}  % Literal typing of ä, ö, ü, etc.
\RequirePackage{amsmath,amssymb}
\RequirePackage{hyperref}
\RequirePackage{array}           % Define new columntypes with \newcolumntype
\RequirePackage[table]{xcolor}   % Colour table cells with \cellcolor
\RequirePackage{styles/drawings}

\newcommand{\om}{{$\omega$}}
\newcommand{\aom}{{$a^\omega$}}

\newcommand{\Bp}{{$B^\prime$}}

% Table column: fixed width, horizontally, vertically centered
\newcolumntype{C}[1]{>{\centering\let\newline\\\arraybackslash\hspace{0pt}}m{#1}}

% Table spacings
{\renewcommand{\arraystretch}{1.15}
\renewcommand{\tabcolsep}{0.1cm}

\endinput % Mandatory
