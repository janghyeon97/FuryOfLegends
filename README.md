# Fury of Legends - AOS Multiplayer Game (Unreal Engine 5)

**Fury of Legends**는 Unreal Engine 5 기반으로 제작된 네트워크 멀티플레이 AOS 게임입니다.  

## 포트폴리오 문서 
1. ![포트폴리오 PDF](Docs/[FuryOfLegends]_언리얼 엔진5 포트폴리오_최장현.gif)
2. [![포트폴리오 영상](http://img.youtube.com/vi/0hS6p3nxGZA/0.jpg)](https://youtu.be/0hS6p3nxGZA?t=0s)

---

## 주요 특징

### 트리 기반 아이템 상점 시스템
- 트리 형태의 아이템 조합 구조
- BFS 기반 탐색 UI 구현

### 상속 기반 능력 시스템
- 캐릭터 고유 능력 및 스킬 실행 처리
- 능력 강화, 쿨다운, 이펙트, 피격 판정 등 통합 관리
- 데이터 테이블 기반 캐릭터 고유의 능력치 관리

### 네트워크 멀티플레이 환경
- 플레이어 상태, 캐릭터, 아이템 등 핵심 데이터 동기화
- 서버에서는 판정 처리, 클라이언트에서는 능력 사용으로 인한 이펙트(파티클, 사운드)만 처리

### 데미지 연출 및 군중 제어 시스템
- 객체 풀링을 활용한 최적화된 `CrowdControlManager` 설계

### 유연한 UI 구성
- RichTextBlock 기반 동적 텍스트 렌더링
- HTML 유사 문법 파싱 및 런타임 수식 계산 기능 구현 (`<calc=MaxHealth*0.1+5>`)

---

## 사용 기술

- **Unreal Engine 5**
- **C++**
- **UMG / Common UI**
- **Net Replication / RPC**
- **Expression Evaluator (수식 파서)**

---

## 디렉터리 구조
- /Characters               // 플레이어 및 미니언 캐릭터
- /Game                     // GameMode, GameState, PlayerState
- /Item                     // 아이템, 아이템 데이터, 상점 UI
- /Components               // StatComponent, ActionStatComponent 등
- /UI                       // UMG 기반 위젯
- /CrowdControls            // 상태이상 처리 모듈



---

## 시연 영상
> 추후 업로드 예정입니다.

---

## 문의

궁금한 점이나 협업 제안이 있다면 아래 이메일로 연락해 주세요.

> janghyen9712@gmail.com
