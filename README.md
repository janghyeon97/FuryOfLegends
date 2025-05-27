⚔️ Fury of Legends - AOS Multiplayer Game (Unreal Engine 5)
Fury of Legends는 언리얼 엔진 5를 기반으로 제작된 네트워크 멀티플레이 AOS(Aeon of Strife) 스타일 게임입니다. 본 프로젝트는 기획, 프로그래밍, UI 및 시스템 구현 등 전체 개발 과정을 직접 주도하며 게임 클라이언트 개발자로서의 실력을 검증하고 성장하기 위해 제작되었습니다.

🎮 주요 특징
트리 기반 아이템 상점 시스템
트리 형태의 아이템 조합 구조와 BFS 기반 탐색 UI 구현
트랜잭션 기반 구매 처리 및 실시간 능력치 반영

상속 기반 능력 시스템
캐릭터 고유 능력 및 스킬 실행 처리
능력 강화, 쿨다운, 이펙트, 피격 판정 처리 등 포함

네트워크 멀티플레이 환경 구현
Server-Client 구조 기반의 안정적인 복제(Replication) 처리
플레이어 상태, 캐릭터, 아이템 등 핵심 데이터 동기화

표현력 있는 데미지 연출과 군중 제어 시스템
실시간 피격 이펙트 및 상태이상 적용 (Stun, Snare, Slow 등)
커스텀 CrowdControlManager를 통한 객체 풀링 최적화

유연한 UI 구성
RichTextBlock을 활용한 동적 텍스트 표현
HTML 유사 문법 파싱 및 런타임 수식 계산 기능 포함

🛠️ 사용 기술
Unreal Engine 5
C++
UMG, Common UI
Net Replication / RPC
Expression Evaluator (수식 파서)

📁 디렉터리 구조
/Characters       // 플레이어 및 미니언 캐릭터
/Game             // GameMode, GameState, PlayerState
/Item             // 아이템, 아이템 데이터, 상점 UI
/Components       // StatComponent, ActionStatComponent 등
/UI               // UMG 기반 위젯
/CrowdControls    // 상태이상 처리 모듈
