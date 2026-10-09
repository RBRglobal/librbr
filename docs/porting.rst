.. Copyright (c) 2026 RBR Ltd.
.. SPDX-License-Identifier: Apache-2.0

Porting
=======

The library makes a few assumptions
about the nature and capacity
of the underlying platform.
These are fairly fundamental
and porting to a platform
where these assumptions do not hold true
may require significant effort.

Integer Sizes
-------------

The library represents all dates/times
as milliseconds since the Unix epoch (1970-01-01T00:00:00.000Z).
These are stored as 64-bit signed integers.

Endianness
----------

The library assumes that the host is little-endian.
