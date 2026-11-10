from cachetools import TTLCache



discovery_reference_cache = TTLCache(maxsize=1, ttl=300)

