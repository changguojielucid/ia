#pragma once


#define SAFE_REMOVAL( __PP) if ( __PP ) { delete __PP ; __PP = nullptr; }