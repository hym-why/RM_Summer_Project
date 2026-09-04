#!/usr/bin/env python3
"""Desktop simulation for the two-sensor line-follow state machine."""

from dataclasses import dataclass
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "Core" / "Inc" / "project_config.h"


def read_define(name: str) -> int:
    text = CONFIG.read_text(encoding="utf-8")
    match = re.search(rf"^#define\s+{name}\s+([^\s/]+)", text, re.MULTILINE)
    if not match:
        raise RuntimeError(f"Missing configuration value: {name}")
    value = match.group(1).rstrip("uU").strip("()")
    return int(value)


BASE_SPEED = read_define("LINE_BASE_SPEED")
TURN_SPEED = read_define("LINE_TURN_OUTER_SPEED")
HINT_CONFIRM_MS = read_define("LINE_HINT_CONFIRM_MS")
HINT_VALID_MS = read_define("LINE_HINT_VALID_MS")
DEFAULT_DIRECTION = read_define("LINE_BLIND_DEFAULT_DIRECTION")
STATE_CONFIRM_MS = read_define("LINE_STATE_CONFIRM_MS")
TURN_MIN_HOLD_MS = read_define("LINE_TURN_MIN_HOLD_MS")
CENTER_CONFIRM_MS = read_define("LINE_CENTER_CONFIRM_MS")
TURN_TIMEOUT_MS = read_define("LINE_TURN_TIMEOUT_MS")
PERIOD_MS = read_define("LINE_CONTROL_PERIOD_MS")


@dataclass(frozen=True)
class Output:
    display: int
    left_pwm: int
    right_pwm: int
    active_turn: int
    last_hint: int


class LineController:
    def __init__(self, initial_pattern: int = 3) -> None:
        self.turn_direction = 0
        self.last_hint = 0
        self.turn_started_ms = 0
        self.last_hint_ms = 0
        self.lost_started_ms = 0
        self.center_started_ms = 0
        self.candidate_started_ms = 0
        self.hint_candidate_started_ms = 0
        self.stable_pattern = initial_pattern
        self.candidate_pattern = initial_pattern
        self.hint_candidate_pattern = initial_pattern
        self.lost_active = False

    @staticmethod
    def turn_output(direction: int) -> tuple[int, int]:
        return (0, TURN_SPEED) if direction < 0 else (TURN_SPEED, 0)

    def update_hint(self, pattern: int, now: int) -> None:
        if pattern not in (1, 2):
            self.hint_candidate_pattern = pattern
            self.hint_candidate_started_ms = now
            return

        if pattern != self.hint_candidate_pattern:
            self.hint_candidate_pattern = pattern
            self.hint_candidate_started_ms = now
            return

        if now - self.hint_candidate_started_ms >= HINT_CONFIRM_MS:
            self.last_hint = -1 if pattern == 1 else 1
            self.last_hint_ms = now

    def start_or_update_turn(self, requested: int, now: int) -> tuple[int, int]:
        if self.turn_direction == 0 or (
            self.turn_direction != requested
            and now - self.turn_started_ms >= TURN_MIN_HOLD_MS
        ):
            self.turn_direction = requested
            self.turn_started_ms = now
        return self.turn_output(self.turn_direction)

    def step(self, raw_pattern: int, now: int) -> Output:
        self.update_hint(raw_pattern, now)

        if raw_pattern != self.candidate_pattern:
            self.candidate_pattern = raw_pattern
            self.candidate_started_ms = now
        elif (
            raw_pattern != self.stable_pattern
            and now - self.candidate_started_ms >= STATE_CONFIRM_MS
        ):
            self.stable_pattern = raw_pattern

        if self.stable_pattern == 1:
            self.center_started_ms = 0
            self.lost_active = False
            self.last_hint = -1
            self.last_hint_ms = now
            left, right = self.start_or_update_turn(-1, now)
        elif self.stable_pattern == 2:
            self.center_started_ms = 0
            self.lost_active = False
            self.last_hint = 1
            self.last_hint_ms = now
            left, right = self.start_or_update_turn(1, now)
        elif self.stable_pattern == 3:
            self.lost_active = False
            if (
                self.turn_direction != 0
                and now - self.turn_started_ms < TURN_MIN_HOLD_MS
            ):
                self.center_started_ms = 0
                left, right = self.turn_output(self.turn_direction)
            else:
                if self.center_started_ms == 0:
                    self.center_started_ms = now
                if (
                    self.turn_direction != 0
                    and now - self.center_started_ms < CENTER_CONFIRM_MS
                ):
                    left, right = self.turn_output(self.turn_direction)
                else:
                    self.turn_direction = 0
                    left, right = BASE_SPEED, BASE_SPEED
        else:
            self.center_started_ms = 0
            if not self.lost_active:
                self.lost_active = True
                self.lost_started_ms = now
                if self.last_hint != 0 and now - self.last_hint_ms <= HINT_VALID_MS:
                    self.turn_direction = self.last_hint
                else:
                    self.turn_direction = DEFAULT_DIRECTION
                self.turn_started_ms = now

            if (
                self.turn_direction != 0
                and now - self.lost_started_ms < TURN_TIMEOUT_MS
            ):
                left, right = self.turn_output(self.turn_direction)
            else:
                self.turn_direction = 0
                left, right = 0, 0

        return Output(
            self.stable_pattern,
            left,
            right,
            self.turn_direction,
            self.last_hint,
        )


def simulate(segments: list[tuple[int, int]]) -> list[tuple[int, int, Output]]:
    controller = LineController(segments[0][0])
    samples: list[tuple[int, int, Output]] = []
    now = 0
    for pattern, duration_ms in segments:
        for _ in range(0, duration_ms, PERIOD_MS):
            samples.append((now, pattern, controller.step(pattern, now)))
            now += PERIOD_MS
    return samples


def transitions(samples: list[tuple[int, int, Output]]) -> list[tuple[int, int, Output]]:
    result = []
    previous = None
    for sample in samples:
        signature = (sample[2].display, sample[2].left_pwm, sample[2].right_pwm)
        if signature != previous:
            result.append(sample)
            previous = signature
    return result


def has_output(samples: list[tuple[int, int, Output]], left: int, right: int) -> bool:
    return any(output.left_pwm == left and output.right_pwm == right for _, _, output in samples)


def run_case(name: str, segments: list[tuple[int, int]], checks) -> None:
    samples = simulate(segments)
    checks(samples)
    print(f"PASS {name}")
    for now, raw, output in transitions(samples):
        print(
            f"  t={now:4d} ms raw={raw} display={output.display} "
            f"motors=({output.left_pwm},{output.right_pwm}) "
            f"turn={output.active_turn:+d} hint={output.last_hint:+d}"
        )


def main() -> None:
    print(
        f"config base={BASE_SPEED} turn={TURN_SPEED} period={PERIOD_MS}ms "
        f"hint={HINT_CONFIRM_MS}ms timeout={TURN_TIMEOUT_MS}ms"
    )

    def straight_checks(samples):
        assert has_output(samples, BASE_SPEED, BASE_SPEED)

    def left_hint_checks(samples):
        assert has_output(samples, 0, TURN_SPEED)
        assert samples[-1][2].left_pwm == BASE_SPEED
        assert samples[-1][2].right_pwm == BASE_SPEED

    def right_hint_checks(samples):
        assert has_output(samples, TURN_SPEED, 0)
        assert samples[-1][2].left_pwm == BASE_SPEED
        assert samples[-1][2].right_pwm == BASE_SPEED

    def default_checks(samples):
        assert has_output(samples, 0, TURN_SPEED)

    def timeout_checks(samples):
        assert samples[-1][2].left_pwm == 0
        assert samples[-1][2].right_pwm == 0

    run_case("straight", [(3, 300)], straight_checks)
    run_case("brief-left-hint-then-lost", [(3, 200), (1, 10), (0, 300), (3, 300)], left_hint_checks)
    run_case("brief-right-hint-then-lost", [(3, 200), (2, 10), (0, 300), (3, 300)], right_hint_checks)
    run_case("blind-default-left", [(3, 200), (0, 300), (3, 300)], default_checks)
    run_case("lost-timeout-stop", [(3, 200), (0, 1300)], timeout_checks)


if __name__ == "__main__":
    main()
