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

\RequirePackage{styles/thesis}
\RequirePackage{styles/drawings}

\endinput % Mandatory

