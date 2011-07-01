PRAGMA journal_mode = MEMORY;
PRAGMA locking_mode = EXCLUSIVE;
PRAGMA synchronous = OFF;
PRAGMA temp_store = MEMORY;
-- PRAGMA temp_store_directory = '/tmp';

-- remove coincs when H1+H2 are the only instruments on

DELETE FROM
        coinc_event
WHERE
        instruments == "H1,H2";

-- remove coincs when H1+H2 are the only participants or when H2+L1 are the
-- only participants when H1+H2+L1 are on

DELETE FROM
        coinc_ringdown
WHERE
        ifos == "H1,H2"
        OR (
                ifos == "H2,L1"
                AND coinc_event_id IN (
                        SELECT
                                coinc_event_id
                        FROM
                                coinc_event
                        WHERE
                                instruments == "H1,H2,L1"
                )
        );

-- remove unused rows from the coinc_event and coinc_ringdown tables

DELETE FROM
        coinc_event
WHERE
        coinc_event_id NOT IN (
                SELECT
                        coinc_event_id
                FROM
                        coinc_ringdown
        );

DELETE FROM
        coinc_ringdown
WHERE
        coinc_event_id NOT IN (
                SELECT
                        coinc_event_id
                FROM
                        coinc_event
        );

-- remove unused rows from the coinc_event_map table

DELETE FROM
        coinc_event_map
WHERE
        coinc_event_id NOT IN (
                SELECT
                        coinc_event_id
                FROM
                        coinc_event
        );

PRAGMA journal_mode = MEMORY;
PRAGMA locking_mode = EXCLUSIVE;
PRAGMA synchronous = OFF;
PRAGMA temp_store = MEMORY;
-- PRAGMA temp_store_directory = '/tmp';

SELECT
        "Number of coincs before clustering: " || count(*)
FROM
        coinc_event;

--
-- construct a look-up table of playground/non-playground state
--

CREATE TEMPORARY TABLE is_playground AS
        SELECT
                coinc_ringdown.coinc_event_id AS coinc_event_id,
                -- is this a zero-lag coinc, and did the last playground
                -- segment start less than 600 seconds prior to it?
                NOT EXISTS (SELECT * FROM time_slide WHERE time_slide.time_slide_id == coinc_event.time_slide_id AND time_slide.offset != 0) AND ((coinc_ringdown.start_time - 729273613) % 6370) < 600 AS is_playground
        FROM
                coinc_ringdown
                JOIN coinc_event ON (
                        coinc_event.coinc_event_id == coinc_ringdown.coinc_event_id
                );
CREATE INDEX tmpindex1 ON is_playground (coinc_event_id);

--
-- create a look-up table of info required for clustering
--

CREATE TEMPORARY TABLE _cluster_info_ AS
        SELECT
                coinc_event.coinc_event_id AS coinc_event_id,
                (coinc_event.time_slide_id || ";" || coinc_event.instruments || ";" || is_playground.is_playground) AS category,
                (coinc_ringdown.start_time - (SELECT MIN(start_time) FROM coinc_ringdown)) + 1e-9 * coinc_ringdown.start_time_ns AS start_time,
                coinc_ringdown.snr AS snr
        FROM
                coinc_event
                JOIN is_playground ON (
                        is_playground.coinc_event_id == coinc_event.coinc_event_id
                )
                JOIN coinc_ringdown ON (
                        coinc_ringdown.coinc_event_id == coinc_event.coinc_event_id
                );
DROP INDEX tmpindex1;
DROP TABLE is_playground;
CREATE INDEX tmpindex1 ON _cluster_info_ (coinc_event_id);
CREATE INDEX tmpindex2 ON _cluster_info_ (category, start_time, snr);

--
-- delete coincs that are within 10 s of coincs with higher SNR in the same
-- category
--

DELETE FROM
        coinc_event
WHERE
        EXISTS (
                SELECT
                        *
                FROM
                        _cluster_info_ AS _cluster_info_a_
                        JOIN _cluster_info_ AS _cluster_info_b_ ON (
                                _cluster_info_b_.category == _cluster_info_a_.category
                                AND (_cluster_info_b_.start_time BETWEEN _cluster_info_a_.start_time - 10.0 AND _cluster_info_a_.start_time + 10.0)
                                AND _cluster_info_b_.snr > _cluster_info_a_.snr
                        )
                WHERE
                        _cluster_info_a_.coinc_event_id == coinc_event.coinc_event_id
        );
DROP INDEX tmpindex1;
DROP INDEX tmpindex2;
DROP TABLE _cluster_info_;

SELECT
        "Number of coincs after clustering: " || count(*)
FROM
        coinc_event;

--
-- delete unused coinc_ringdown rows
--

DELETE FROM
        coinc_ringdown
WHERE
        coinc_event_id NOT IN (
                SELECT
                        coinc_event_id
                FROM
                        coinc_event
        );

--
-- delete unused coinc_event_map rows
--

DELETE FROM
        coinc_event_map
WHERE
        coinc_event_id NOT IN (
                SELECT
                        coinc_event_id
                FROM
                        coinc_event
        );

--
-- delete unused sngl_ringdown rows
--

DELETE FROM
        sngl_ringdown
WHERE
        event_id NOT IN (
                SELECT
                        event_id
                FROM
                        coinc_event_map
                WHERE
                        table_name == 'sngl_ringdown'
        );

--
-- shrink the file
--

VACUUM;